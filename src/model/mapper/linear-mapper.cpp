#include "linear-mapper.hpp"

namespace wibot {

// ============================================================================
// LinearMapper 模板实现
// ============================================================================

template <u8 CHANNELS>
LinearMapper<CHANNELS>::LinearMapper(SyncPipeline<i16>& upstream, const Config& config)
    : _upstream(upstream), _config(config) {
}

template <u8 CHANNELS>
f32 LinearMapper<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 无效通道返回0
    }

    // 获取上游值并实时处理
    i16 input = _upstream.getValue(channel);
    return _mapLinear(static_cast<f32>(input));
}

template <u8 CHANNELS>
void LinearMapper<CHANNELS>::reset() {
    // 无状态映射器，无需重置，但需要重置上游
    _upstream.reset();
}

template <u8 CHANNELS>
void LinearMapper<CHANNELS>::update() {
    // 无状态映射器，只需要更新上游
    _upstream.update();
}

template <u8 CHANNELS>
void LinearMapper<CHANNELS>::updateConfig(const Config& config) {
    _config = config;
}

template <u8 CHANNELS>
f32 LinearMapper<CHANNELS>::_mapLinear(f32 input) const {
    if (_config.inputMax == _config.inputMin) {
        return _config.outputMin;
    }

    f32 ratio  = (input - _config.inputMin) / (_config.inputMax - _config.inputMin);
    f32 result = _config.outputMin + ratio * (_config.outputMax - _config.outputMin);

    // 限制输出范围（如果启用）
    if (_config.clampOutput) {
        if (result < _config.outputMin) result = _config.outputMin;
        if (result > _config.outputMax) result = _config.outputMax;
    }

    return result;
}

}  // namespace wibot