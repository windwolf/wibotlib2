#pragma once

#include "model.hpp"
#include "type.hpp"

namespace wibot {

template <typename TIN, u8 CHANNELS_IN = 1, typename TOUT = TIN>
class NToOnePipelineAdapter : public SyncPipeline<TOUT> {
   public:
    /**
     * @brief 构造单通道适配器
     * 
     * @param upstream 上游单通道管道
     * @param enableUpstreamControl 是否自动控制上游管道的reset/update
     */
    NToOnePipelineAdapter(MultiChannelPipeline<TIN, CHANNELS_IN>& upstream, u8 channel = 0,
                          bool enableUpstreamControl = true)
        : _upstream(upstream), _channel(channel), _enableUpstreamControl(enableUpstreamControl) {
    }

    void setUpstreamControl(bool enable) {
        _enableUpstreamControl = enable;
    }

    bool getUpstreamControl() const {
        return _enableUpstreamControl;
    }

    TOUT getValue() const override {
        return _upstream.getValue(_channel);
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
    MultiChannelPipeline<TIN, CHANNELS_IN>& _upstream;
    u8                                      _channel;
    bool                                    _enableUpstreamControl;
};

template <typename TIN, typename TOUT = TIN, u8 CHANNELS_OUT = 1>
class OneToNPipelineAdapter : public MultiChannelPipeline<TOUT, CHANNELS_OUT> {
   public:
    /**
     * @brief 构造广播适配器
     * 
     * @param upstream 上游单通道管道
     * @param enableUpstreamControl 是否自动控制上游管道的reset/update
     */
    OneToNPipelineAdapter(SyncPipeline<TIN>& upstream, bool enableUpstreamControl = true)
        : _upstream(upstream), _enableUpstreamControl(enableUpstreamControl) {
    }

    void setUpstreamControl(bool enable) {
        _enableUpstreamControl = enable;
    }

    bool getUpstreamControl() const {
        return _enableUpstreamControl;
    }

    TOUT getValue(u8 channel) const override {
        return static_cast<TOUT>(_upstream.getValue());
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
    bool               _enableUpstreamControl;
};

}  // namespace wibot
