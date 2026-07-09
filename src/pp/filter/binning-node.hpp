#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/binning.hpp"

namespace wibot {

template <typename T>
    requires SupportArithmetic<T>
class BinningNode : public INode {
   public:
    using Config = typename Binning<T>::Config;

    struct Inputs {
        In<T> value;
    } inputs;

    struct Outputs {
        Out<u32> binIndex;
    } outputs;

    explicit BinningNode(Config& config) : _config(config), _binning(config) {
    }

    bool ready() override {
        return inputs.value.bound() && configValid();
    }

    void process() override {
        if (!outputs.binIndex.bound()) {
            return;
        }
        outputs.binIndex.ref() = _binning.process(inputs.value.get());
    }

    void reset() override {
        _binning.reset();
    }

   private:
    bool configValid() const {
        return _config.binCount > 0U && _config.boundaries != nullptr;
    }

    Config&    _config;
    Binning<T> _binning;
};

}  // namespace wibot
