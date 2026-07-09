#include "lowpass-node.hpp"

namespace wibot {

LowpassNode::LowpassNode(Config& config) : _config(config), _filter(config) {
}

bool LowpassNode::ready() {
    return inputs.x.bound() && IIR::isConfigValid(_config);
}

void LowpassNode::process() {
    if (!outputs.y.bound()) {
        return;
    }
    outputs.y.ref() = _filter.filter(inputs.x.get());
}

void LowpassNode::reset() {
    _filter.reset();
}

}  // namespace wibot
