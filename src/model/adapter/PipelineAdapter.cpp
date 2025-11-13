#include "PipelineAdapter.hpp"

namespace wibot {

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::PipelineAdapter(SyncPipeline<TIN>& upstream)
    : _upstream(upstream), _enableUpstreamControl(true) {
    // 初始化通道映射数组，默认所有输出通道映射到输入通道0
    for (u8 i = 0; i < CHANNELS_OUT; ++i) {
        _channelIndex[i] = 0;
    }
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::~PipelineAdapter() {
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
Result PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::mapChannel(u8 channelOut, u8 channelIn) {
    if (channelOut >= CHANNELS_OUT) {
        return Result::kInvalidParameter;
    }

    _channelIndex[channelOut] = channelIn;
    return Result::kOk;
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
Result PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::mapChannels(const u8 channelMap[CHANNELS_OUT]) {
    if (channelMap == nullptr) {
        return Result::kInvalidParameter;
    }

    for (u8 i = 0; i < CHANNELS_OUT; ++i) {
        _channelIndex[i] = channelMap[i];
    }

    return Result::kOk;
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
void PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::setUpstreamControl(bool enableUpstreamControl) {
    _enableUpstreamControl = enableUpstreamControl;
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
bool PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::getUpstreamControl() const {
    return _enableUpstreamControl;
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
TOUT PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::getValue(u8 channel) const {
    if (channel >= CHANNELS_OUT) {
        return TOUT{};  // 返回默认值
    }
    return static_cast<TOUT>(_upstream.getValue(_channelIndex[channel]));
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
void PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::reset() {
    if (_enableUpstreamControl) {
        _upstream.reset();
    }
}

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
void PipelineAdapter<TIN, CHANNELS_OUT, TOUT>::update() {
    if (_enableUpstreamControl) {
        _upstream.update();
    }
}

}  // namespace wibot
