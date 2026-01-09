#pragma once

#include "../pipeline.hpp"
#include "dsp/controller/slope.hpp"

namespace wibot::pp {

template <typename T>
    requires SupportArithmetic<T>
class SlopeTrajectoryNode : public INode {
   public:
    using Config = typename dsp::SlopeTrajectory<T>::Config;

    struct Inputs {
        In<T> setPoint;
    } inputs;

    struct Outputs {
        Out<T> output;
    } outputs;

    explicit SlopeTrajectoryNode(Config& config) : _trajectory(config) {
    }

    bool ready() override {
        return inputs.setPoint.bound() && outputs.output.bound();
    }

    void process() override {
        outputs.output.ref() = _trajectory.update(inputs.setPoint.get());
    }

    void reset() override {
        _trajectory.reset();
    }

    void setInitialValue(T value) {
        _trajectory.setInitialValue(value);
    }

   private:
    dsp::SlopeTrajectory<T> _trajectory;
};

}  // namespace wibot::pp
