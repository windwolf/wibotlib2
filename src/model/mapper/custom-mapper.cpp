#include "custom-mapper.hpp"

namespace wibot {

// ============================================================================
// CustomMapper 模板实现
// ============================================================================

template <typename TIn, typename TOut, u8 CHANNELS>
CustomMapper<TIn, TOut, CHANNELS>::CustomMapper(SyncPipeline<TIn>& upstream, const Config& config)
    : _upstream(upstream), _config(config) {
    // 验证配置有效性
    if (!isConfigValid(config)) {
        // 如果配置无效，提供一个默认的恒等映射函数
        _config.mappingFunc = [](TIn input, u8 /*channel*/) -> TOut {
            return static_cast<TOut>(input);
        };
    }
}

template <typename TIn, typename TOut, u8 CHANNELS>
TOut CustomMapper<TIn, TOut, CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return TOut{};  // 无效通道返回默认值
    }

    // 获取上游值并直接调用映射函数
    TIn input = _upstream.getValue(channel);
    return _config.mappingFunc(input, channel);
}

template <typename TIn, typename TOut, u8 CHANNELS>
void CustomMapper<TIn, TOut, CHANNELS>::reset() {
    // 无状态映射器，无需重置，但需要重置上游
    _upstream.reset();
}

template <typename TIn, typename TOut, u8 CHANNELS>
void CustomMapper<TIn, TOut, CHANNELS>::update() {
    // 无状态映射器，只需要更新上游
    _upstream.update();
}

template <typename TIn, typename TOut, u8 CHANNELS>
void CustomMapper<TIn, TOut, CHANNELS>::updateConfig(const Config& config) {
    if (isConfigValid(config)) {
        _config = config;
    }
    // 如果配置无效，保持原有配置不变
}

template <typename TIn, typename TOut, u8 CHANNELS>
bool CustomMapper<TIn, TOut, CHANNELS>::isConfigValid(const Config& config) {
    // 检查映射函数是否有效
    return static_cast<bool>(config.mappingFunc);
}

}  // namespace wibot