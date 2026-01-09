#pragma once

#include "../pipeline.hpp"
#include "dsp/controller/trapezoid.hpp"

namespace wibot {

class TrapezoidTrajectoryNode : public INode {
   public:
    using Config = TrapezoidTrajectory::Config;
    using Phase  = TrapezoidTrajectory::Phase;

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
    TrapezoidTrajectory _trajectory;
};

}  // namespace wibot
