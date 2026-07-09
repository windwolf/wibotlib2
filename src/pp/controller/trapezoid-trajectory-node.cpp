#include "trapezoid-trajectory-node.hpp"

namespace wibot {

TrapezoidTrajectoryNode::TrapezoidTrajectoryNode(Config& config) : _trajectory(config) {
}

bool TrapezoidTrajectoryNode::ready() {
    return inputs.setPoint.bound();
}

void TrapezoidTrajectoryNode::process() {
    if (!outputs.position.bound() && !outputs.velocity.bound() && !outputs.phase.bound()) {
        return;
    }

    f32 pos                = _trajectory.update(inputs.setPoint.get());
    if (outputs.position.bound()) {
        outputs.position.ref() = pos;
    }
    if (outputs.velocity.bound()) {
        outputs.velocity.ref() = _trajectory.getVelocity();
    }
    if (outputs.phase.bound()) {
        outputs.phase.ref() = _trajectory.getPhase();
    }
}

void TrapezoidTrajectoryNode::reset() {
    _trajectory.reset();
}

void TrapezoidTrajectoryNode::setInitialValue(f32 value) {
    _trajectory.setInitialValue(value);
}

}  // namespace wibot
