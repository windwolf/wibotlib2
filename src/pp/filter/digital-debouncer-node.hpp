#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/digital-debouncer.hpp"

namespace wibot::pipeline {

template <u8 CHANNELS>
    requires(CHANNELS <= 32)
class DigitalDebouncerNode : public INode {
   public:
    using Core   = dsp::DigitalDebouncer<CHANNELS>;
    using Config = typename Core::Config;

    struct Inputs {
        In<u32> rawMask;
        In<u32> tickMs;
    } inputs;

    struct Outputs {
        Out<u32> status;
    } outputs;

    explicit DigitalDebouncerNode(Config& config) : _core(config) {
    }

    bool ready() override {
        return inputs.rawMask.bound() && inputs.tickMs.bound() && outputs.status.bound();
    }

    void process() override {
        _core.updateRawValues(inputs.rawMask.get());
        outputs.status.ref() = _core.process(inputs.tickMs.get());
    }

    void reset() override {
        const u32 tick = inputs.tickMs.bound() ? inputs.tickMs.get() : 0U;
        _core.reset(tick);
    }

    u32 getChannel(u8 channel) const {
        return _core.getChannel(channel) ? 1U : 0U;
    }

   private:
    Core _core;
};

}  // namespace wibot::pipeline
