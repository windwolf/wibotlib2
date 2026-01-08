#pragma once

#include "model.hpp"
#include "type.hpp"

namespace wibot {

template <typename TIN, u8 CHANNELS_IN = 1, typename TOUT = TIN>
class ChannelMux : public SyncPipeline<TOUT> {
   public:
    /**
     * @brief 构造单通道适配器
     * 
     * @param upstream 上游单通道管道
     * @param enableUpstreamControl 是否自动控制上游管道的reset/update
     */
    ChannelMux(MultiChannelPipeline<TIN, CHANNELS_IN>& upstream, u8 channel = 0,
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
class FanOutPipeline : public MultiChannelPipeline<TOUT, CHANNELS_OUT> {
   public:
    /**
     * @brief 构造广播适配器
     * 
     * @param upstream 上游单通道管道
     * @param enableUpstreamControl 是否自动控制上游管道的reset/update
     */
    FanOutPipeline(SyncPipeline<TIN>& upstream, bool enableUpstreamControl = true)
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

template <typename TIN, u8 CHANNELS_IN = 1, typename TOUT = TIN,
        typename Combiner = TOUT (*)(const MultiChannelPipeline<TIN, CHANNELS_IN>&)>
class ChannelReducer : public SyncPipeline<TOUT> {
   public:
    /**
     * @brief 构造多路合并适配器
     *
     * @param upstream 上游多通道管道
    * @param combiner 自定义合并函数，签名为 `TOUT func(const MultiChannelPipeline<TIN, CHANNELS_IN>&)`
     * @param enableUpstreamControl 是否自动控制上游管道的reset/update
     */
    ChannelReducer(MultiChannelPipeline<TIN, CHANNELS_IN>& upstream, Combiner combiner,
                   bool enableUpstreamControl = true)
        : _upstream(upstream), _combiner(combiner), _enableUpstreamControl(enableUpstreamControl) {
    }

    void setUpstreamControl(bool enable) {
        _enableUpstreamControl = enable;
    }

    bool getUpstreamControl() const {
        return _enableUpstreamControl;
    }

    void setCombiner(Combiner combiner) {
        _combiner = combiner;
    }

    TOUT getValue() const override {
        return _combiner(_upstream);
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
    Combiner                                _combiner;
    bool                                    _enableUpstreamControl;
};

}  // namespace wibot
