#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/key-scaner.hpp"

namespace wibot::pp {

template <u8 CHANNELS>
class KeyScanerNode : public INode {
   public:
    using Core     = dsp::KeyScaner<CHANNELS>;
    using Config   = typename Core::Config;
    using KeyEvent = typename Core::KeyEvent;

    struct Inputs {
        In<u32> pinStatusMask;
        In<u32> tickMs;
    } inputs;

    struct Outputs {
        Out<KeyEvent> events[CHANNELS];
    } outputs;

    explicit KeyScanerNode(Config& config) : _core(config) {
    }

    bool ready() override {
        return inputs.pinStatusMask.bound() && inputs.tickMs.bound() && inputs.channel.bound() &&
               outputs.event.bound();
    }

    void process() override {
        _core.scan(inputs.pinStatusMask.get(), inputs.tickMs.get());
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            outputs.events[ch].ref() = _core.getLastEvent(ch);
        }
    }

    void reset() override {
        _core.reset();
    }

   private:
    Core _core;
};

}  // namespace wibot::pp
