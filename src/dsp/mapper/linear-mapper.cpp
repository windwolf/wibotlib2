#include "dsp/mapper/linear-mapper.hpp"

namespace wibot::dsp {

LinearMapper::LinearMapper(const Config& cfg) : _config(cfg) {
}

f32 LinearMapper::process(f32 input) {
    if (_config.inputMax == _config.inputMin) {
        return _config.outputMin;
    }

    f32 ratio  = (input - _config.inputMin) / (_config.inputMax - _config.inputMin);
    f32 result = _config.outputMin + ratio * (_config.outputMax - _config.outputMin);

    // 限制输出范围（如果启用）
    if (_config.clampOutput) {
        if (result < _config.outputMin) {
            result = _config.outputMin;
        }
        if (result > _config.outputMax) {
            result = _config.outputMax;
        }
    }

    return result;
}

bool LinearMapper::isConfigValid(const Config& cfg) {
    return cfg.inputMax > cfg.inputMin;
}

}  // namespace wibot::dsp
