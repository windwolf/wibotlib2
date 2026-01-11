#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/key-scaner.hpp"

namespace wibot {

template <u8 CHANNELS>
class KeyScanerNode : public INode {
   public:
    using Core     = KeyScaner<CHANNELS>;
    using Config   = typename Core::Config;
    using KeyEvent = KeyEvent;

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
        if (!inputs.pinStatusMask.bound() || !inputs.tickMs.bound()) {
            return false;
        }
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            if (!outputs.events[ch].bound()) {
                return false;
            }
        }
        return true;
    }

    void process() override {
        _core.scan(inputs.pinStatusMask.get(), inputs.tickMs.get());
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            outputs.events[ch].ref() = _core.getCurrentEvent(ch);
        }
    }

    void reset() override {
        _core.reset();
    }

   private:
    Core _core;
};

}  // namespace wibot
