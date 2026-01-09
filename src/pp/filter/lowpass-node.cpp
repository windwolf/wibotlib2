#include "lowpass-node.hpp"

namespace wibot::pp {

LowpassNode::LowpassNode(Config& config) : _config(config), _filter(config) {
}

bool LowpassNode::ready() {
    return inputs.x.bound() && outputs.y.bound() && dsp::Lowpass::isConfigValid(_config);
}

void LowpassNode::process() {
    outputs.y.ref() = _filter.filter(inputs.x.get());
}

void LowpassNode::reset() {
    _filter.reset();
}

}  // namespace wibot::pp
