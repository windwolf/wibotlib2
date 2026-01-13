#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/iir.hpp"

namespace wibot {

class LowpassNode : public INode {
   public:
    using Config = IIR::Config;

    struct Inputs {
        In<f32> x;
    } inputs;

    struct Outputs {
        Out<f32> y;
    } outputs;

    explicit LowpassNode(Config& config);

    bool ready() override;

    void process() override;

    void reset() override;

   private:
    Config& _config;
    IIR     _filter;
};

}  // namespace wibot
