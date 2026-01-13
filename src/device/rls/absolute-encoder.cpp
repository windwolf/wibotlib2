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
    dispThreshold_               = std::max(1.0f, speedThreshold * _config.samplePeriod);

    // 配置高速段低通滤波器
    // f_c_high = 4 / (2π * N * T_s)
    _highLpConfig.samplePeriod = _config.samplePeriod;
    _highLpConfig.wrapValue    = 0.0f;
    f32 tauHigh                = _config.samplePeriod * _config.highTrackingCycles / 4.0f;
    _highLpConfig.cutoffFreq   = k1_2PI / tauHigh;
    _highLp.applyConfig();
    _highLp.reset();

    // 计算最小有效采样周期：使低速段能在单个采样周期内提供1 tick的位移
    f32 minEffectiveSamplePeriod_ = 1.0f / _config.minSpeed;
    // 配置低速段低通滤波器
    // f_c_low = 4 / (2π * M * T_有效采样)
    _lowLpConfig.samplePeriod     = minEffectiveSamplePeriod_;
    _lowLpConfig.wrapValue        = 0.0f;
    f32 tauLow                    = minEffectiveSamplePeriod_ * _config.lowTrackingCycles / 4.0f;
    _lowLpConfig.cutoffFreq       = k1_2PI / tauLow;
    _lowLp.applyConfig();
    _lowLp.reset();
}

void AbsoluteEncoder::reset(f32 position, f32 speed) {
    position_                = position;
    lastValue_               = static_cast<u32>(position);
    speed_                   = speed;
    accumulatedDisplacement_ = static_cast<i64>(position);
    lowSpeedAccumulator_     = 0;
    lowSpeedTimeAccum_       = 0.0f;
    _highLp.reset();
    _lowLp.reset();
}

i32 AbsoluteEncoder::calculateDisplacement_(u32 currentValue) {
    // 计算编码值变化量，考虑套圈
    i32 delta = static_cast<i32>(currentValue) - static_cast<i32>(lastValue_);

    // 处理套圈：如果增量超过范围的一半，说明跨越了0/wrapRange边界
    const i32 range = static_cast<i32>(_config.resolution);
    if (delta < -range / 2) {
        delta += range;
    } else if (delta > range / 2) {
        delta -= range;
    }

    return delta;
}

f32 AbsoluteEncoder::getMixingFactor_(f32 displacement) {
    // 基于位移的混合因子计算
    // 当位移 < dispThreshold 时，进行过渡
    // 过渡区间宽度为 dispThreshold * 0.2

    f32 transitionWidth = dispThreshold_ * 0.2f;
    f32 lowerBound      = dispThreshold_ - transitionWidth;
    f32 upperBound      = dispThreshold_ + transitionWidth;

    if (displacement < lowerBound) {
        return 0.0f;  // 全低速
    } else if (displacement > upperBound) {
        return 1.0f;  // 全高速
    } else {
        // 线性插值过渡
        return (displacement - lowerBound) / (upperBound - lowerBound);
    }
}

f32 AbsoluteEncoder::calculateHighSpeedVelocity_(i32 displacement, f32 samplePeriod) {
    // 高速段：差分法
    f32 rawSpeed = static_cast<f32>(displacement) / samplePeriod;
    return _highLp.filter(rawSpeed);
}

f32 AbsoluteEncoder::calculateLowSpeedVelocity_(i64 accumulatedDispl) {
    // 低速段：累计法
    // 仅当累计位移达到1 tick时才计算速度
    i32 intDispl = static_cast<i32>(accumulatedDispl);

    if (intDispl != 0 && lowSpeedTimeAccum_ > 0.0f) {
        f32 rawSpeed      = static_cast<f32>(intDispl) / lowSpeedTimeAccum_;
        f32 filteredSpeed = _lowLp.filter(rawSpeed);
        lowSpeedAccumulator_ -= intDispl;  // 消耗已计算的位移
        lowSpeedTimeAccum_ = 0.0f;         // 清零已使用的时间
        return filteredSpeed;
    }

    return _lowLp.filter(0.0f);
}

void AbsoluteEncoder::update(u32 value, bool wrapped) {
    update(value, _config.samplePeriod, wrapped);
}

void AbsoluteEncoder::update(u32 value, f32 samplePeriod, bool wrapped) {
    // 处理编码器值折叠
    if (wrapped) {
        value = value % _config.resolution;
    }

    // 计算位移
    i32 displacement = calculateDisplacement_(value);

    // 更新绝对位置
    position_ += displacement;
    accumulatedDisplacement_ += displacement;
    lowSpeedAccumulator_ += displacement;
    lowSpeedTimeAccum_ += samplePeriod;

    lastValue_ = value;

    // 基于位移的高低速切换策略
    f32 absDispl = std::abs(static_cast<f32>(displacement));

    // 如果单周期位移 < 1 tick，必定使用低速段
    if (absDispl < 1.0f) {
        speed_ = calculateLowSpeedVelocity_(lowSpeedAccumulator_);
    } else {
        // 计算混合因子
        f32 mixFactor = getMixingFactor_(absDispl);

        // 计算高低速段速度
        f32 speedHigh = calculateHighSpeedVelocity_(displacement, samplePeriod);
        f32 speedLow  = calculateLowSpeedVelocity_(lowSpeedAccumulator_);

        // 混合输出
        speed_ = speedLow + mixFactor * (speedHigh - speedLow);
    }

    LOG_D("Position: %.2f, Speed: %.2f, Displacement: %d", position_, speed_, displacement);
}

f32 AbsoluteEncoder::getAngular() const {
    // 将位置转换为弧度
    // 角度 = (position / resolution) * 2π
    f32 revolutions = position_ / static_cast<f32>(_config.resolution);
    return revolutions * 2.0f * 3.14159265f;
}

f32 AbsoluteEncoder::getAngularSpeed() const {
    // 将速度转换为弧度/s
    // 角速度 = (speed / resolution) * 2π
    f32 rps = speed_ / static_cast<f32>(_config.resolution);
    return rps * 2.0f * 3.14159265f;
}

}  // namespace wibot
