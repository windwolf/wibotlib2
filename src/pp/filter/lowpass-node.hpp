#pragma once

#include "../pipeline.hpp"
#include "dsp/filter/lowpass.hpp"

namespace wibot {

class LowpassNode : public INode {
   public:
    using Config = Lowpass::Config;

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
    Lowpass _filter;
};

}  // namespace wibot
