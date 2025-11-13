#include "piecewise-linear-mapper.hpp"
#include <cmath>

namespace wibot {

// ============================================================================
// PiecewiseLinearMapper 模板实现
// ============================================================================

template <u8 CHANNELS>
PiecewiseLinearMapper<CHANNELS>::PiecewiseLinearMapper(SyncPipeline<i16>& upstream,
                                                       const Config&      config)
    : _upstream(upstream), _config(config) {
    // 构造时验证配置有效性
    if (!isConfigValid(config)) {
        // 如果配置无效，可以选择抛出异常或使用默认配置
        // 这里暂时保留原配置，实际使用时需要注意验证
    }
}

template <u8 CHANNELS>
f32 PiecewiseLinearMapper<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 无效通道返回0
    }

    // 获取上游值并实时处理
    i16 input = _upstream.getValue(channel);
    return _mapPiecewiseLinear(static_cast<f32>(input));
}

template <u8 CHANNELS>
void PiecewiseLinearMapper<CHANNELS>::reset() {
    // 无状态映射器，无需重置，但需要重置上游
    _upstream.reset();
}

template <u8 CHANNELS>
void PiecewiseLinearMapper<CHANNELS>::update() {
    // 无状态映射器，只需要更新上游
    _upstream.update();
}

template <u8 CHANNELS>
void PiecewiseLinearMapper<CHANNELS>::updateConfig(const Config& config) {
    if (isConfigValid(config)) {
        _config = config;
    }
    // 如果配置无效，保持原有配置不变
}

template <u8 CHANNELS>
bool PiecewiseLinearMapper<CHANNELS>::isConfigValid(const Config& config) {
    // 检查基本参数
    if (config.inputPoints == nullptr || config.outputPoints == nullptr ||
        config.segmentCount == 0) {
        return false;
    }

    // 检查输入控制点是否按升序排列
    for (u8 i = 0; i < config.segmentCount; ++i) {
        if (config.inputPoints[i] >= config.inputPoints[i + 1]) {
            return false;  // 控制点未按升序排列
        }
    }
    return true;
}

template <u8 CHANNELS>
f32 PiecewiseLinearMapper<CHANNELS>::_mapPiecewiseLinear(f32 input) const {
    // 查找输入值所在的分段
    u8 segmentIndex = _findSegmentIndex(input);

    if (segmentIndex == INVALID_SEGMENT) {
        // 输入值超出所有分段范围
        if (_config.enableExtrapolation) {
            // 进行外推
            if (input < _config.inputPoints[0]) {
                // 在第一个分段之前，使用第一个分段的斜率外推
                return _interpolateInSegment(input, 0);
            } else {
                // 在最后一个分段之后，使用最后一个分段的斜率外推
                return _interpolateInSegment(input, _config.segmentCount - 1);
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
    f32 result = _interpolateInSegment(input, segmentIndex);

    // 如果启用了输出钳位，限制结果范围
    if (_config.clampOutput) {
        f32 minOutput = _config.outputPoints[0];
        f32 maxOutput = _config.outputPoints[0];

        // 找到输出范围
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

template <u8 CHANNELS>
u8 PiecewiseLinearMapper<CHANNELS>::_findSegmentIndex(f32 input) const {
    // 检查是否在范围内
    if (input < _config.inputPoints[0] || input > _config.inputPoints[_config.segmentCount]) {
        return INVALID_SEGMENT;
    }

    // 查找输入值所在的分段
    for (u8 i = 0; i < _config.segmentCount; ++i) {
        if (input >= _config.inputPoints[i] && input <= _config.inputPoints[i + 1]) {
            return i;
        }
    }

    return INVALID_SEGMENT;
}

template <u8 CHANNELS>
f32 PiecewiseLinearMapper<CHANNELS>::_interpolateInSegment(f32 input, u8 segmentIndex) const {
    // 确保分段索引有效
    if (segmentIndex >= _config.segmentCount) {
        return NAN;  // 无效分段
    }

    f32 x0 = _config.inputPoints[segmentIndex];
    f32 x1 = _config.inputPoints[segmentIndex + 1];
    f32 y0 = _config.outputPoints[segmentIndex];
    f32 y1 = _config.outputPoints[segmentIndex + 1];

    // 防止除零错误
    if (x1 == x0) {
        return y0;  // 如果输入点相同，返回第一个输出值
    }

    // 线性插值公式：y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
    f32 ratio = (input - x0) / (x1 - x0);
    return y0 + (y1 - y0) * ratio;
}

}  // namespace wibot