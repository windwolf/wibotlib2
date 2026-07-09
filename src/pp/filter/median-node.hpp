#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/median.hpp"

namespace wibot {

template <typename T>
class MedianNode : public INode {
   public:
    using Config = typename Median<T>::Config;

    struct Inputs {
        In<T> x;
    } inputs;

    struct Outputs {
        Out<T> y;
    } outputs;

    explicit MedianNode(Config& config) : _config(config), _filter(config) {
    }

    bool ready() override {
        return inputs.x.bound() && Median<T>::isConfigValid(_config);
    }

    void process() override {
        if (!outputs.y.bound()) {
            return;
        }
        outputs.y.ref() = _filter.filter(inputs.x.get());
    }

    void reset() override {
        _filter.reset();
    }

   private:
    Config&   _config;
    Median<T> _filter;
};

}  // namespace wibot
