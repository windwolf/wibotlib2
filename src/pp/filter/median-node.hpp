#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/median.hpp"

namespace wibot::pp {

template <typename T>
class MedianNode : public INode {
   public:
    using Config = typename dsp::Median<T>::Config;

    struct Inputs {
        In<T> x;
    } inputs;

    struct Outputs {
        Out<T> y;
    } outputs;

    explicit MedianNode(Config& config) : _config(config), _filter(config) {
    }

    bool ready() override {
        return inputs.x.bound() && outputs.y.bound() && dsp::Median<T>::isConfigValid(_config);
    }

    void process() override {
        outputs.y.ref() = _filter.filter(inputs.x.get());
    }

    void reset() override {
        _filter.reset();
    }

   private:
    Config&        _config;
    dsp::Median<T> _filter;
};

}  // namespace wibot::pp
