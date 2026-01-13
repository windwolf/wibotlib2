#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/digital-debouncer.hpp"

namespace wibot {

template <u8 CHANNELS>
    requires(CHANNELS <= 32)
class DigitalDebouncerNode : public INode {
   public:
    using Core   = DigitalDebouncer<CHANNELS>;
    using Config = typename Core::Config;

    struct Inputs {
        In<u32> rawMask;
        In<u32> samplePeriodMs;
    } inputs;

    struct Outputs {
        Out<u32> status;
    } outputs;

    explicit DigitalDebouncerNode(Config& config) : _core(config) {
    }

    bool ready() override {
        return inputs.rawMask.bound() && inputs.samplePeriodMs.bound() && outputs.status.bound();
    }

    void process() override {
        _core.updateRawValues(inputs.rawMask.get());
        outputs.status.ref() = _core.process(inputs.samplePeriodMs.get());
    }

    void reset() override {
        _core.reset();
    }

    u32 getChannel(u8 channel) const {
        return _core.getChannel(channel) ? 1U : 0U;
    }

   private:
    Core _core;
};

}  // namespace wibot
