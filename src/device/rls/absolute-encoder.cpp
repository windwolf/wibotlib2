#include "absolute-encoder.hpp"
#include "math.hpp"
#include <algorithm>
#include <cmath>
#include "logger.hpp"

LOGGER("enc")

namespace wibot {

void AbsoluteEncoder::applyConfig() {
    // 计算位移阈值：(minSpeed + speedDynRange * 10%) * samplePeriod，最小值1 tick

    constexpr f32 SPEED_THRESHOLD_FACTOR = 0.1f;
    f32           speedDynRange          = _config.maxSpeed - _config.minSpeed;
    f32           speedThreshold = _config.minSpeed + speedDynRange * SPEED_THRESHOLD_FACTOR;
    _dispThreshold               = std::max(1.0f, speedThreshold * _config.samplePeriod);

    // 配置高速段低通滤波器
    // f_c_high = 4 / (2π * N * T_s)
    _highLpConfig.samplePeriod = _config.samplePeriod;
    _highLpConfig.wrapValue    = 0.0f;
    f32 tauHigh                = _config.samplePeriod * _config.highTrackingCycles / 4.0f;
    _highLpConfig.cutoffFreq   = k1_2PI / tauHigh;
    _highLp.applyConfig();
    _highLp.reset();

    // 计算最小有效采样周期：使低速段能在单个采样周期内提供1 tick的位移
    f32 minEffectiveSamplePeriod = 1.0f / _config.minSpeed;
    // 配置低速段低通滤波器
    // f_c_low = 4 / (2π * M * T_有效采样)
    _lowLpConfig.samplePeriod    = minEffectiveSamplePeriod;
    _lowLpConfig.wrapValue       = 0.0f;
    f32 tauLow                   = minEffectiveSamplePeriod * _config.lowTrackingCycles / 4.0f;
    _lowLpConfig.cutoffFreq      = k1_2PI / tauLow;
    _lowLp.applyConfig();
    _lowLp.reset();
}

void AbsoluteEncoder::reset(i32 position, f32 speed) {
    _position            = position;
    _lastValue           = static_cast<u32>(position);
    _speed               = speed;
    _lowSpeedAccumulator = 0;
    _lowSpeedTimeAccum   = 0.0f;
    _highLp.reset();
    _lowLp.reset();
}

i32 AbsoluteEncoder::_calculateDisplacement(u32 currentValue) {
    // 计算编码值变化量，考虑套圈
    // 先在无符号域做减法（利用模运算），再转为有符号
    i32 delta = static_cast<i32>(currentValue - _lastValue);

    // 处理套圈：如果增量超过范围的一半，说明跨越了0/wrapRange边界
    if (_config.inputWrapRange != 0) {
        const i32 range = static_cast<i32>(_config.inputWrapRange);
        if (delta < -range / 2) {
            delta += range;
        } else if (delta > range / 2) {
            delta -= range;
        }
    }

    return delta;
}

f32 AbsoluteEncoder::_getMixingFactor(f32 displacement) {
    // 基于位移的混合因子计算
    // 当位移 < dispThreshold 时，进行过渡
    // 过渡区间宽度为 dispThreshold * 0.2

    f32 transitionWidth = _dispThreshold * 0.2f;
    f32 lowerBound      = _dispThreshold - transitionWidth;
    f32 upperBound      = _dispThreshold + transitionWidth;

    if (displacement < lowerBound) {
        return 0.0f;  // 全低速
    } else if (displacement > upperBound) {
        return 1.0f;  // 全高速
    } else {
        // 线性插值过渡
        return (displacement - lowerBound) / (upperBound - lowerBound);
    }
}

f32 AbsoluteEncoder::_calculateHighSpeedVelocity(i32 displacement, f32 samplePeriod) {
    // 高速段：差分法
    f32 rawSpeed = static_cast<f32>(displacement) / samplePeriod;
    return _highLp.filter(rawSpeed);
}

f32 AbsoluteEncoder::_calculateLowSpeedVelocity(i32 accumulatedDispl) {
    // 低速段：累计法
    // 仅当累计位移达到1 tick时才计算速度
    i32 intDispl = accumulatedDispl;

    if (intDispl != 0 && _lowSpeedTimeAccum > 0.0f) {
        f32 rawSpeed      = static_cast<f32>(intDispl) / _lowSpeedTimeAccum;
        f32 filteredSpeed = _lowLp.filter(rawSpeed);
        _lowSpeedAccumulator -= intDispl;  // 消耗已计算的位移
        _lowSpeedTimeAccum = 0.0f;         // 清零已使用的时间
        return filteredSpeed;
    }

    return _lowLp.filter(0.0f);
}

void AbsoluteEncoder::update(u32 value) {
    update(value, _config.samplePeriod);
}

void AbsoluteEncoder::update(u32 value, f32 samplePeriod) {
    // 处理输入折叠：inputWrapRange==0 表示无需折叠
    if (_config.inputWrapRange != 0) {
        value = value % _config.inputWrapRange;
    }

    // 计算位移
    i32 displacement = _calculateDisplacement(value);
    _position += displacement;
    _lastValue = value;

    // 基于位移的高低速切换策略
    auto absDispl = std::abs(displacement);

    // 高速段：位移大于等于阈值时，直接用高速路径，清空低速累积
    if (_dispThreshold > 0.0f && absDispl >= _dispThreshold) {
        _lowSpeedAccumulator = 0;
        _lowSpeedTimeAccum   = 0.0f;
        _speed               = _calculateHighSpeedVelocity(displacement, samplePeriod);
        return;
    }

    // 低速/过渡段：累积位移与时间供低速估计
    _lowSpeedAccumulator += displacement;
    _lowSpeedTimeAccum += samplePeriod;

    // 如果单周期位移为0，纯低速估计
    if (absDispl == 0) {
        _speed = _calculateLowSpeedVelocity(_lowSpeedAccumulator);
        return;
    }

    // 过渡区：按位移权重混合高低速输出
    f32 mixFactor = _getMixingFactor(static_cast<f32>(absDispl));
    f32 speedHigh = _calculateHighSpeedVelocity(displacement, samplePeriod);
    f32 speedLow  = _calculateLowSpeedVelocity(_lowSpeedAccumulator);
    _speed        = speedLow + mixFactor * (speedHigh - speedLow);
}

f32 AbsoluteEncoder::getAngular() const {
    // 将位置转换为弧度
    // 角度 = (position / resolution) * 2π
    f32 revolutions = _position / static_cast<f32>(_config.resolution);
    return revolutions * k2PI;
}

f32 AbsoluteEncoder::getAngularSpeed() const {
    // 将速度转换为弧度/s
    // 角速度 = (speed / resolution) * 2π
    f32 rps = _speed / static_cast<f32>(_config.resolution);
    return rps * k2PI;
}

}  // namespace wibot
