#include "offset-calibrator-node.hpp"

namespace wibot {

bool OffsetCalibratorNode::ready() {
    return inputs.sample.bound() && outputs.offset.bound();
}

void OffsetCalibratorNode::process() {
    if (inputs.requestReset.bound() && inputs.requestReset.get()) {
        _calibrator.reset();
    }

    const bool allowSample = !inputs.enableSample.bound() || inputs.enableSample.get();
    if (allowSample) {
        _calibrator.addSample(inputs.sample.get());
    }

    bool calculated = false;
    if (inputs.requestCalculate.bound() && inputs.requestCalculate.get()) {
        calculated = _calibrator.calculate();
    }

    if (outputs.offset.bound()) {
        outputs.offset.ref() = _calibrator.getOffset();
    }
    if (outputs.sampleCount.bound()) {
        outputs.sampleCount.ref() = _calibrator.getSampleCount();
    }
    if (outputs.calculated.bound()) {
        outputs.calculated.ref() = calculated;
    }
}

void OffsetCalibratorNode::reset() {
    _calibrator.reset();
}

}  // namespace wibot
