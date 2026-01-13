#include "dsp/filter/iir.hpp"
#include "math.hpp"
#include <cmath>

namespace wibot {

IIR::IIR(const Config& cfg) : _config(cfg), _y_last(0.0f), _first(true) {
    _updateCoefficients();
}

bool IIR::applyConfig() {
    if (!isConfigValid(_config)) {
        return false;
    }
    _updateCoefficients();
    return true;
}

f32 IIR::filter(f32 input) {
    if (_first) {
        _y_last = input;
        _first  = false;
    } else {
        if (_config.wrapValue <= 0.0f) {
            // 非周期数据
            _y_last = _alpha * input + _one_minus_alpha * _y_last;
        } else {
            // 周期数据（如角度）
            const f32 diff        = input - _y_last;
            const f32 wrappedDiff = _wrap(diff, _config.wrapValue);
            _y_last               = _wrap(_y_last + _alpha * wrappedDiff, _config.wrapValue);
        }
    }
    return _y_last;
}

f32 IIR::filter(f32 input, f32 samplePeriod) {
    const f32 alpha           = _computeAlpha(samplePeriod);
    const f32 one_minus_alpha = 1.0f - alpha;

    if (_first) {
        _y_last = input;
        _first  = false;
    } else {
        if (_config.wrapValue <= 0.0f) {
            // 非周期数据
            _y_last = alpha * input + one_minus_alpha * _y_last;
        } else {
            // 周期数据（如角度）
            const f32 diff        = input - _y_last;
            const f32 wrappedDiff = _wrap(diff, _config.wrapValue);
            _y_last               = _wrap(_y_last + alpha * wrappedDiff, _config.wrapValue);
        }
    }
    return _y_last;
}

void IIR::reset() {
    _y_last = 0.0f;
    _first  = true;
}

bool IIR::isConfigValid(const Config& cfg) {
    if (cfg.samplePeriod <= 0.0f || cfg.cutoffFreq <= 0.0f) {
        return false;
    }
    const f32 fs = 1.0f / cfg.samplePeriod;
    if (cfg.cutoffFreq >= fs * 0.5f) {  // 奈奎斯特定理
        return false;
    }
    if (cfg.wrapValue < 0.0f) {
        return false;
    }
    return true;
}

void IIR::_updateCoefficients() {
    _alpha           = _computeAlpha(_config.samplePeriod);
    _one_minus_alpha = 1.0f - _alpha;
}

f32 IIR::_computeAlpha(f32 samplePeriod) const {
    const f32 omega = 2.0f * kPI * _config.cutoffFreq * samplePeriod;
    return omega / (1.0f + omega);
}

f32 IIR::_wrap(f32 x, f32 w) {
    return x - 2.0f * w * std::floor((x + w) / (2.0f * w));
}

}  // namespace wibot
