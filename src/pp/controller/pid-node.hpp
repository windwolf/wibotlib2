#pragma once

#include "../pipeline.hpp"
#include "dsp/controller/pid.hpp"

namespace wibot {

class PidNode : public INode {
   public:
    struct Inputs {
        In<f32> measurement;
        In<f32> setPoint;
    } inputs;

    struct Outputs {
        Out<f32> output;
    } outputs;

    explicit PidNode(Pid::Config& config);

    bool ready() override;

    void process() override;

    void reset() override;

    void resetIntegrator();

   private:
    Pid _pid;
};

}  // namespace wibot
