#pragma once

#include "../pipeline.hpp"
#include "dsp/mapper/piecewise-linear-mapper.hpp"

namespace wibot::pipeline {

class PiecewiseLinearMapperNode : public INode {
   public:
    using Config = dsp::PiecewiseLinearMapper::Config;

    struct Inputs {
        In<f32> x;
    } inputs;

    struct Outputs {
        Out<f32> y;
    } outputs;

    explicit PiecewiseLinearMapperNode(Config& config);

    bool ready() override;

    void process() override;

    void reset() override;

   private:
    Config&                    _config;
    dsp::PiecewiseLinearMapper _mapper;
};

}  // namespace wibot::pipeline
