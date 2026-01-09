#include "dsp/controller/trapezoid.hpp"
#include <algorithm>
#include <cmath>

namespace wibot {

TrapezoidTrajectory::TrapezoidTrajectory(Config& config) : _config(config) {
}

void TrapezoidTrajectory::reset() {
    _state.output           = 0.0f;
    _state.setPoint         = 0.0f;
    _state.velocity         = 0.0f;
    _state.phase            = Phase::kIdle;
    _state.startPosition    = 0.0f;
    _state.targetPosition   = 0.0f;
    _state.accelDistance    = 0.0f;
    _state.decelDistance    = 0.0f;
    _state.constantDistance = 0.0f;
    _state.accelTime        = 0.0f;
    _state.constantTime     = 0.0f;
    _state.decelTime        = 0.0f;
    _state.phaseTimer       = 0.0f;
    _state.direction        = 1.0f;
}

void TrapezoidTrajectory::setInitialValue(f32 value) {
    _state.output   = clampValue(value);
    _state.setPoint = _state.output;
    _state.phase    = Phase::kIdle;
    _state.velocity = 0.0f;
}

f32 TrapezoidTrajectory::update(f32 setPoint) {
    if (std::abs(setPoint - _state.setPoint) > 1e-9f) {
        _state.setPoint = setPoint;
        calculateTrajectory(_state.output, setPoint);
    }

    switch (_state.phase) {
        case Phase::kIdle:
        case Phase::kCompleted:
            break;
        case Phase::kAcceleration:
            accelStep();
            break;
        case Phase::kConstant:
            constantStep();
            break;
        case Phase::kDeceleration:
            decelStep();
            break;
    }
    _state.output = clampValue(_state.output);
    return _state.output;
}

f32 TrapezoidTrajectory::getVelocity() const {
    return _state.velocity;
}
TrapezoidTrajectory::Phase TrapezoidTrajectory::getPhase() const {
    return _state.phase;
}

bool TrapezoidTrajectory::isReached(f32 tolerance) const {
    return _state.phase == Phase::kCompleted &&
           std::abs(_state.output - _state.setPoint) <= tolerance;
}

void TrapezoidTrajectory::accelStep() {
    _state.phaseTimer += _config.sampleTime;
    if (_state.phaseTimer >= _state.accelTime) {
        _state.phase      = Phase::kConstant;
        _state.phaseTimer = 0.0f;
        _state.velocity   = _config.maxVelocity * _state.direction;
        _state.output     = _state.startPosition + _state.accelDistance * _state.direction;
    } else {
        f32 t           = _state.phaseTimer;
        _state.velocity = _config.acceleration * t * _state.direction;
        _state.output =
            _state.startPosition + 0.5f * _config.acceleration * t * t * _state.direction;
    }
}

void TrapezoidTrajectory::constantStep() {
    _state.phaseTimer += _config.sampleTime;
    if (_state.phaseTimer >= _state.constantTime) {
        _state.phase      = Phase::kDeceleration;
        _state.phaseTimer = 0.0f;
        _state.output     = _state.startPosition +
                        (_state.accelDistance + _state.constantDistance) * _state.direction;
    } else {
        f32 t         = _state.phaseTimer;
        _state.output = _state.startPosition +
                        (_state.accelDistance + _config.maxVelocity * t) * _state.direction;
    }
}

void TrapezoidTrajectory::decelStep() {
    _state.phaseTimer += _config.sampleTime;
    if (_state.phaseTimer >= _state.decelTime) {
        _state.phase    = Phase::kCompleted;
        _state.velocity = 0.0f;
        _state.output   = _state.targetPosition;
    } else {
        f32 t             = _state.phaseTimer;
        f32 decelDistance = _config.maxVelocity * t - 0.5f * _config.deceleration * t * t;
        _state.velocity   = (_config.maxVelocity - _config.deceleration * t) * _state.direction;
        _state.output =
            _state.startPosition +
            (_state.accelDistance + _state.constantDistance + decelDistance) * _state.direction;
    }
}

void TrapezoidTrajectory::calculateTrajectory(f32 start, f32 target) {
    _state.startPosition  = start;
    _state.targetPosition = target;

    f32 totalDistance = target - start;
    _state.direction  = (totalDistance >= 0) ? 1.0f : -1.0f;
    totalDistance     = std::abs(totalDistance);

    if (totalDistance < 1e-6f) {
        _state.phase    = Phase::kCompleted;
        _state.velocity = 0.0f;
        _state.output   = target;
        return;
    }

    f32 accelDistance = (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.acceleration);
    f32 decelDistance = (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.deceleration);

    if (accelDistance + decelDistance > totalDistance) {
        f32 peakVelocity =
            std::sqrt(2.0f * totalDistance * _config.acceleration * _config.deceleration /
                      (_config.acceleration + _config.deceleration));

        _state.accelDistance    = (peakVelocity * peakVelocity) / (2.0f * _config.acceleration);
        _state.decelDistance    = (peakVelocity * peakVelocity) / (2.0f * _config.deceleration);
        _state.constantDistance = 0.0f;

        _state.accelTime    = peakVelocity / _config.acceleration;
        _state.constantTime = 0.0f;
        _state.decelTime    = peakVelocity / _config.deceleration;
    } else {
        _state.accelDistance    = accelDistance;
        _state.decelDistance    = decelDistance;
        _state.constantDistance = totalDistance - accelDistance - decelDistance;

        _state.accelTime    = _config.maxVelocity / _config.acceleration;
        _state.constantTime = _state.constantDistance / _config.maxVelocity;
        _state.decelTime    = _config.maxVelocity / _config.deceleration;
    }

    _state.phase      = Phase::kAcceleration;
    _state.phaseTimer = 0.0f;
    _state.velocity   = 0.0f;
}

f32 TrapezoidTrajectory::clampValue(f32 value) const {
    if (std::isnan(value) || std::isinf(value)) {
        return 0.0f;
    }
    const f32 maxValue = 1e6f;
    const f32 minValue = -1e6f;
    return std::max(minValue, std::min(maxValue, value));
}

}  // namespace wibot
