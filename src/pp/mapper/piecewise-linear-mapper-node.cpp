#include "piecewise-linear-mapper-node.hpp"

namespace wibot {

PiecewiseLinearMapperNode::PiecewiseLinearMapperNode(Config& config)
    : _config(config), _mapper(config) {
}

bool PiecewiseLinearMapperNode::ready() {
    return inputs.x.bound() && PiecewiseLinearMapper::isConfigValid(_config);
}

void PiecewiseLinearMapperNode::process() {
    if (!outputs.y.bound()) {
        return;
    }
    outputs.y.ref() = _mapper.map(inputs.x.get());
}

void PiecewiseLinearMapperNode::reset() {
}

}  // namespace wibot
