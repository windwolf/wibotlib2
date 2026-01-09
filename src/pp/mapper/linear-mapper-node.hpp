#pragma once

#include "../pipeline.hpp"
#include "dsp/mapper/linear-mapper.hpp"

namespace wibot::pp {

class LinearMapperNode : public INode {
   public:
    using Config = dsp::LinearMapper::Config;

    struct Inputs {
        In<f32> x;
    } inputs;

    struct Outputs {
        Out<f32> y;
    } outputs;

    explicit LinearMapperNode(Config& config);

    bool ready() override;

    void process() override;

    void reset() override;

   private:
    Config&           _config;
    dsp::LinearMapper _mapper;
};

}  // namespace wibot::pp
