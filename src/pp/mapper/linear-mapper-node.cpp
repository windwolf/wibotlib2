#include "linear-mapper-node.hpp"

namespace wibot {

LinearMapperNode::LinearMapperNode(Config& config) : _config(config), _mapper(config) {
}

bool LinearMapperNode::ready() {
    return inputs.x.bound() && LinearMapper::isConfigValid(_config);
}

void LinearMapperNode::process() {
    if (!outputs.y.bound()) {
        return;
    }
    outputs.y.ref() = _mapper.process(inputs.x.get());
}

void LinearMapperNode::reset() {
}

}  // namespace wibot
