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
        In<u32> samplePeriodMs;
    } inputs;

    struct Outputs {
        Out<KeyEvent> events[CHANNELS];
    } outputs;

    explicit KeyScanerNode(Config& config) : _core(config) {
    }

    bool ready() override {
        return inputs.pinStatusMask.bound() && inputs.samplePeriodMs.bound();
    }

    void process() override {
        // 前置检查太耗费, 不如不检查
        // bool outputRequested = false;
        // for (u8 ch = 0; ch < CHANNELS; ++ch) {
        //     outputRequested = outputRequested || outputs.events[ch].bound();
        // }
        // if (!outputRequested) {
        //     return;
        // }

        _core.scan(inputs.pinStatusMask.get(), inputs.samplePeriodMs.get());
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            if (outputs.events[ch].bound()) {
                outputs.events[ch].ref() = _core.getCurrentEvent(ch);
            }
        }
    }

    void reset() override {
        _core.reset();
    }

   private:
    Core _core;
};

}  // namespace wibot
