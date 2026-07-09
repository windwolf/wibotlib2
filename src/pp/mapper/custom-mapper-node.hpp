#pragma once

#include "../pipeline.hpp"
#include "dsp/mapper/custom-mapper.hpp"

namespace wibot {

template <typename TIn, typename TOut>
class CustomMapperNode : public INode {
   public:
    using Config = typename CustomMapper<TIn, TOut>::Config;

    struct Inputs {
        In<TIn> x;
    } inputs;

    struct Outputs {
        Out<TOut> y;
    } outputs;

    explicit CustomMapperNode(Config& config) : _mapper(config) {
    }

    bool ready() override {
        return inputs.x.bound() && _mapper.isConfigValid();
    }

    void process() override {
        if (!outputs.y.bound()) {
            return;
        }
        outputs.y.ref() = _mapper.map(inputs.x.get());
    }

    void reset() override {
    }

   private:
    CustomMapper<TIn, TOut> _mapper;
};

}  // namespace wibot
