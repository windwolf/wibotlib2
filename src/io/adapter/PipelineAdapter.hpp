#pragma once

#include "model.hpp"
#include "../base/type.hpp"

namespace wibot {

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
class PipelineAdapter : public SyncPipeline<TOUT> {
   public:
    PipelineAdapter(SyncPipeline<TIN>& upstream)
        : _upstream(upstream), _enableUpstreamControl(true) {
        // 初始化通道映射数组，默认所有输出通道映射到输入通道0
        for (u8 i = 0; i < CHANNELS_OUT; ++i) {
            _channelIndex[i] = 0;
        }
    }

    ~PipelineAdapter() {
    }

    Result mapChannel(u8 channelOut, u8 channelIn) {
        if (channelOut >= CHANNELS_OUT) {
            return Result::kInvalidParameter;
        }
        _channelIndex[channelOut] = channelIn;
        return Result::kOk;
    }

    Result mapChannels(const u8 channelMap[CHANNELS_OUT]) {
        if (channelMap == nullptr) {
            return Result::kInvalidParameter;
        }
        for (u8 i = 0; i < CHANNELS_OUT; ++i) {
            _channelIndex[i] = channelMap[i];
        }
        return Result::kOk;
    }

    void setUpstreamControl(bool enableUpstreamControl) {
        _enableUpstreamControl = enableUpstreamControl;
    }

    bool getUpstreamControl() const {
        return _enableUpstreamControl;
    }

    TOUT getValue(u8 channel) const override {
        if (channel >= CHANNELS_OUT) {
            return TOUT{};  // 返回默认值
        }
        return static_cast<TOUT>(_upstream.getValue(_channelIndex[channel]));
    }

    void reset() override {
        if (_enableUpstreamControl) {
            _upstream.reset();
        }
    }

    void update() override {
        if (_enableUpstreamControl) {
            _upstream.update();
        }
    }

   private:
    SyncPipeline<TIN>& _upstream;
    u8                 _channelIndex[CHANNELS_OUT];
    bool               _enableUpstreamControl;
};

}  // namespace wibot