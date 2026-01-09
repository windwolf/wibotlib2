#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/key-scaner.hpp"

namespace wibot::pipeline {

template <u8 CHANNELS>
class KeyScanerNode : public INode {
   public:
    using Core     = dsp::KeyScaner<CHANNELS>;
    using Config   = typename Core::Config;
    using KeyEvent = typename Core::KeyEvent;

    struct Inputs {
        In<u32> pinStatusMask;
        In<u32> tickMs;
        In<u8>  channel;
    } inputs;

    struct Outputs {
        Out<KeyEvent> event;
        Out<u8>       clickCount;
    } outputs;

    explicit KeyScanerNode(Config& config) : _core(config) {
    }

    bool ready() override {
        return inputs.pinStatusMask.bound() && inputs.tickMs.bound() && inputs.channel.bound() &&
               outputs.event.bound();
    }

    void process() override {
        _core.scan(inputs.pinStatusMask.get(), inputs.tickMs.get());
        const u8 ch         = inputs.channel.get();
        outputs.event.ref() = _core.getLastEvent(ch);
        if (outputs.clickCount.bound()) {
            outputs.clickCount.ref() = _core.getClickCount(ch);
        }
    }

    void reset() override {
        _core.reset();
    }

   private:
    Core _core;
};

}  // namespace wibot::pipeline
