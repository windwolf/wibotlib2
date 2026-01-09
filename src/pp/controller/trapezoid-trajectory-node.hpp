#pragma once

#include "../pipeline.hpp"
#include "dsp/controller/trapezoid.hpp"

namespace wibot::pp {

class TrapezoidTrajectoryNode : public INode {
   public:
    using Config = dsp::TrapezoidTrajectory::Config;
    using Phase  = dsp::TrapezoidTrajectory::Phase;

    struct Inputs {
        In<f32> setPoint;
    } inputs;

    struct Outputs {
        Out<f32>   position;
        Out<f32>   velocity;
        Out<Phase> phase;
    } outputs;

    explicit TrapezoidTrajectoryNode(Config& config);

    bool ready() override;

    void process() override;

    void reset() override;

    void setInitialValue(f32 value);

   private:
    dsp::TrapezoidTrajectory _trajectory;
};

}  // namespace wibot::pp
