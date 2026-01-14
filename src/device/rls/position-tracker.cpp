#include "position-tracker.hpp"
#include "math.hpp"

namespace wibot {

void PositionTracker::update(u32 value) {
    // 处理输入折叠：inputWrapRange==0 表示无需折叠
    if (_config.inputWrapRange != 0) {
        value = value % _config.inputWrapRange;
    }

    // 计算位移
    _lastDisplacement = _calculateDisplacement(value);
    _position += _lastDisplacement;
    _lastValue = value;
}

void PositionTracker::reset(u32 value, i32 position) {
    // 处理输入折叠
    if (_config.inputWrapRange != 0) {
        value = value % _config.inputWrapRange;
    }

    _position         = position;
    _lastValue        = value;
    _lastDisplacement = 0;
}

i32 PositionTracker::_calculateDisplacement(u32 currentValue) {
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

f32 PositionTracker::getAngular() const {
    // 将位置转换为弧度
    // 角度 = (position / resolution) * 2π
    f32 revolutions = _position / static_cast<f32>(_config.resolution);
    return revolutions * k2PI;
}

}  // namespace wibot
