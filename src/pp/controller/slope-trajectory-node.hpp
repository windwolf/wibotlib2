#pragma once

#include "../pipeline.hpp"
#include "dsp/controller/slope.hpp"

namespace wibot {

template <typename T>
    requires SupportArithmetic<T>
class SlopeTrajectoryNode : public INode {
   public:
    using Config = typename SlopeTrajectory<T>::Config;

    struct Inputs {
        In<T>   setPoint;
        In<f32> samplePeriod;  // 采样周期（秒）
    } inputs;

    struct Outputs {
        Out<T> output;
    } outputs;

    explicit SlopeTrajectoryNode(Config& config) : _trajectory(config) {
    }

    bool ready() override {
        return inputs.setPoint.bound() && inputs.samplePeriod.bound() && outputs.output.bound();
    }

    void process() override {
        outputs.output.ref() = _trajectory.update(inputs.setPoint.get(), inputs.samplePeriod.get());
    }

    void reset() override {
        _trajectory.reset();
    }

    void setInitialValue(T value) {
        _trajectory.setInitialValue(value);
    }

   private:
    SlopeTrajectory<T> _trajectory;
};

}  // namespace wibot
