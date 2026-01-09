#include "dsp/controller/pid.hpp"
#include <algorithm>

namespace wibot {

Pid::Pid(const Config& cfg)
    : _config(cfg),
      _integrator(0.0f),
      _prevError(0.0f),
      _differentiator(0.0f),
      _prevMeasurement(0.0f),
      _output(0.0f) {
}

f32 Pid::update(f32 measurement, f32 setPoint) {
    f32 error = setPoint - measurement;

    // 比例项
    f32 pTerm = _config.Kp * error;

    // 积分项
    _integrator += error * _config.sampleTime;
    if (_config.integratorLimitEnable) {
        if (_integrator > _config.integratorLimitMax) {
            _integrator = _config.integratorLimitMax;
        }
        if (_integrator < _config.integratorLimitMin) {
            _integrator = _config.integratorLimitMin;
        }
    }
    f32 iTerm = _config.Ki * _integrator;

    // 微分项：基于测量值（避免微分 kick）
    if (_config.sampleTime > 0.0f && _config.tau > 0.0f) {
        const f32 alpha        = _config.sampleTime / (_config.tau + _config.sampleTime);
        f32       dMeasurement = (measurement - _prevMeasurement) / _config.sampleTime;
        _differentiator        = alpha * dMeasurement + (1.0f - alpha) * _differentiator;
    }
    f32 dTerm = -_config.Kd * _differentiator;

    // 总输出（串并行目前一致）
    _output = pTerm + iTerm + dTerm;

    // 输出限制
    if (_config.outputLimitEnable) {
        if (_output > _config.outputLimitMax) {
            _output = _config.outputLimitMax;
        }
        if (_output < _config.outputLimitMin) {
            _output = _config.outputLimitMin;
        }
    }

    // 更新历史值
    _prevError       = error;
    _prevMeasurement = measurement;

    return _output;
}

void Pid::reset() {
    _integrator      = 0.0f;
    _prevError       = 0.0f;
    _differentiator  = 0.0f;
    _prevMeasurement = 0.0f;
    _output          = 0.0f;
}

void Pid::resetIntegrator() {
    _integrator = 0.0f;
}

}  // namespace wibot
