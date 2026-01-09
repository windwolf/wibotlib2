#include "piecewise-linear-mapper-node.hpp"

namespace wibot {

PiecewiseLinearMapperNode::PiecewiseLinearMapperNode(Config& config)
    : _config(config), _mapper(config) {
}

bool PiecewiseLinearMapperNode::ready() {
    return inputs.x.bound() && outputs.y.bound() && PiecewiseLinearMapper::isConfigValid(_config);
}

void PiecewiseLinearMapperNode::process() {
    outputs.y.ref() = _mapper.map(inputs.x.get());
}

void PiecewiseLinearMapperNode::reset() {
}

}  // namespace wibot
