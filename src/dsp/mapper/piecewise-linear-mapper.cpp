#include "dsp/mapper/piecewise-linear-mapper.hpp"
#include <cmath>

namespace wibot {

PiecewiseLinearMapper::PiecewiseLinearMapper(Config& config) : _config(config) {
}

f32 PiecewiseLinearMapper::map(f32 input) {
    u8 segmentIndex = findSegmentIndex(input);

    if (segmentIndex == INVALID_SEGMENT) {
        // 输入值超出所有分段范围
        if (_config.enableExtrapolation) {
            // 进行外推
            if (input < _config.inputPoints[0]) {
                return interpolateInSegment(input, 0);
            } else {
                return interpolateInSegment(input, _config.segmentCount - 1);
            }
        } else {
            // 不允许外推，返回边界值
            if (input < _config.inputPoints[0]) {
                return _config.outputPoints[0];
            } else {
                return _config.outputPoints[_config.segmentCount];
            }
        }
    }

    // 在有效分段内进行插值
    f32 result = interpolateInSegment(input, segmentIndex);

    // 如果启用了输出钳位，限制结果范围
    if (_config.clampOutput) {
        f32 minOutput = _config.outputPoints[0];
        f32 maxOutput = _config.outputPoints[0];

        for (u8 i = 1; i <= _config.segmentCount; ++i) {
            if (_config.outputPoints[i] < minOutput) {
                minOutput = _config.outputPoints[i];
            }
            if (_config.outputPoints[i] > maxOutput) {
                maxOutput = _config.outputPoints[i];
            }
        }

        if (result < minOutput) result = minOutput;
        if (result > maxOutput) result = maxOutput;
    }

    return result;
}

bool PiecewiseLinearMapper::isConfigValid(const Config& config) {
    if (config.inputPoints == nullptr || config.outputPoints == nullptr ||
        config.segmentCount == 0) {
        return false;
    }

    // 检查输入控制点是否按升序排列
    for (u8 i = 0; i < config.segmentCount; ++i) {
        if (config.inputPoints[i] >= config.inputPoints[i + 1]) {
            return false;
        }
    }
    return true;
}

u8 PiecewiseLinearMapper::findSegmentIndex(f32 input) const {
    if (input < _config.inputPoints[0] || input > _config.inputPoints[_config.segmentCount]) {
        return INVALID_SEGMENT;
    }

    for (u8 i = 0; i < _config.segmentCount; ++i) {
        if (input >= _config.inputPoints[i] && input <= _config.inputPoints[i + 1]) {
            return i;
        }
    }

    return INVALID_SEGMENT;
}

f32 PiecewiseLinearMapper::interpolateInSegment(f32 input, u8 segmentIndex) const {
    if (segmentIndex >= _config.segmentCount) {
        return NAN;
    }

    f32 x0 = _config.inputPoints[segmentIndex];
    f32 x1 = _config.inputPoints[segmentIndex + 1];
    f32 y0 = _config.outputPoints[segmentIndex];
    f32 y1 = _config.outputPoints[segmentIndex + 1];

    if (x1 == x0) {
        return y0;
    }

    // 线性插值：y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
    f32 ratio = (input - x0) / (x1 - x0);
    return y0 + (y1 - y0) * ratio;
}

}  // namespace wibot
