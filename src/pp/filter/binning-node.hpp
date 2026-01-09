#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/binning.hpp"

namespace wibot::pipeline {

template <typename T>
    requires SupportArithmetic<T>
class BinningNode : public INode {
   public:
    using Config = typename dsp::Binning<T>::Config;

    struct Inputs {
        In<T> value;
    } inputs;

    struct Outputs {
        Out<u32> binIndex;
    } outputs;

    explicit BinningNode(Config& config) : _config(config), _binning(config) {
    }

    bool ready() override {
        return inputs.value.bound() && outputs.binIndex.bound() && configValid();
    }

    void process() override {
        outputs.binIndex.ref() = _binning.process(inputs.value.get());
    }

    void reset() override {
        _binning.reset();
    }

   private:
    bool configValid() const {
        return _config.binCount > 0U && _config.boundaries != nullptr;
    }

    Config&         _config;
    dsp::Binning<T> _binning;
};

}  // namespace wibot::pipeline
