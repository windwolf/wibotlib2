#include "pid-node.hpp"

namespace wibot::pp {

PidNode::PidNode(dsp::Pid::Config& config) : _pid(config) {
}

bool PidNode::ready() {
    return inputs.measurement.bound() && inputs.setPoint.bound() && outputs.output.bound();
}

void PidNode::process() {
    outputs.output.ref() = _pid.update(inputs.measurement.get(), inputs.setPoint.get());
}

void PidNode::reset() {
    _pid.reset();
}

void PidNode::resetIntegrator() {
    _pid.resetIntegrator();
}

}  // namespace wibot::pp
