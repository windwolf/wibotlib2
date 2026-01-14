#include "absolute-encoder.hpp"
#include <cmath>

namespace wibot {

void AbsoluteEncoder::update(u32 value) {
    update(value, _config.samplePeriod);
}

void AbsoluteEncoder::update(u32 value, f32 samplePeriod) {
    // 更新位置跟踪器（位置始终更新）
    _posTracker.update(value);

    // 获取位移
    i32 displacement = _posTracker.getLastDisplacement();

    // 累计位移
    _accumulatedDisplacement += displacement;
    _accumulatedTime += samplePeriod;

    // 检查位移是否超过阈值
    i32 absAccumDispl = std::abs(_accumulatedDisplacement);
    if (absAccumDispl >= _config.minDisplacementThreshold) {
        // 位移足够：更新速度估计器
        _velEstimator.update(_accumulatedDisplacement, _accumulatedTime);

        // 重置累计
        _accumulatedDisplacement = 0;
        _accumulatedTime         = 0.0f;
    }
    // 如果位移不足，继续累计，不更新速度估计器
}

void AbsoluteEncoder::reset(u32 value, i32 position, f32 speed) {
    // 重置位置跟踪器
    _posTracker.reset(value, position);

    // 重置速度估计器
    _velEstimator.reset(speed);

    // 重置位移累计
    _accumulatedDisplacement = 0;
    _accumulatedTime         = 0.0f;
}

}  // namespace wibot
