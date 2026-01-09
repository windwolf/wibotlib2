#pragma once

#include "../pipeline.hpp"
#include "dsp/mapper/linear-mapper.hpp"

namespace wibot {

class LinearMapperNode : public INode {
   public:
    using Config = LinearMapper::Config;

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
    Config&      _config;
    LinearMapper _mapper;
};

}  // namespace wibot
