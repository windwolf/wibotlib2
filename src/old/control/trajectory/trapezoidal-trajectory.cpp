//
// Created by zhouj on 2022/12/8.
//

#include "trapezoidal-trajectory.hpp"

#include <cmath>

namespace wibot {
static f32 sign_hard(f32 val) {
    return (std::signbit(val)) ? -1.0f : 1.0f;
}
bool TrapezoidalTrajectory::plan(f32 start_pos, f32 end_pos, f32 start_vel, f32 max_vel,
                                 f32 max_acc, f32 max_dec) {
    f32 dX        = end_pos - start_pos;                         // Distance to travel
    f32 stop_dist = (start_vel * start_vel) / (2.0f * max_dec);  // Minimum stopping distance
    f32 dXstop    = std::copysign(stop_dist, start_vel);         // Minimum stopping displacement
    f32 s         = sign_hard(dX - dXstop);                      // Sign of coast velocity (if any)
    _accRated     = s * max_acc;                                 // Maximum Acceleration (signed)
    _decRated     = -s * max_dec;                                // Maximum Deceleration (signed)
    _velRated     = s * max_vel;                                 // Maximum Velocity (signed)

    // If we start with a speed faster than cruising, then we need to decel instead of accel
    // aka "f64 deceleration move" in the paper
    if ((s * start_vel) > (s * _velRated)) {
        _accRated = -s * max_acc;
    }

    // Time to accel/decel to/from Vr (cruise speed)
    _accTime = (_velRated - start_vel) / _accRated;
    _decTime = -_velRated / _decRated;

    // Integral of velocity ramps over the full accel and decel times to get
    // minimum displacement required to reach cuising speed
    f32 dXmin = 0.5f * _accTime * (_velRated + start_vel) + 0.5f * _decTime * _velRated;

    // Are we displacing enough to reach cruising speed?
    if (s * dX < s * dXmin) {
        // Short move (triangle profile)
        _velRated   = s * std::sqrt(std::max(
                            (_decRated * start_vel * start_vel + 2 * _accRated * _decRated * dX) /
                                (_decRated - _accRated),
                            0.0f));
        _accTime    = std::max(0.0f, (_velRated - start_vel) / _accRated);
        _decTime    = std::max(0.0f, -_velRated / _decRated);
        _cruiseTime = 0.0f;
    } else {
        // Long move (trapezoidal profile)
        _cruiseTime = (dX - dXmin) / _velRated;
    }

    // Fill in the rest of the values used at evaluation-time
    _finalTime = _accTime + _cruiseTime + _decTime;
    _startPos  = start_pos;
    _endPos    = end_pos;
    _startVel  = start_vel;
    _yAccel    = start_pos + start_vel * _accTime +
              0.5f * _accRated * _accTime * _accTime;  // pos at end of accel phase
    _yCruise = _yAccel + _velRated * _cruiseTime;      // pos at end of cruise phase
    return true;
}

TrapezoidalTrajectoryStep TrapezoidalTrajectory::evalByTime(f32 t) {
    TrapezoidalTrajectoryStep trajStep;
    if (t < 0.0f) {  // Initial Condition
        trajStep.Y     = _startPos;
        trajStep.Yd    = _startVel;
        trajStep.Ydd   = 0.0f;
        trajStep.stage = TrapezoidalTrajectoryStage::kInit;
    } else if (t < _accTime) {  // Accelerating
        trajStep.Y     = _startPos + _startVel * t + 0.5f * _accRated * t * t;
        trajStep.Yd    = _startVel + _accRated * t;
        trajStep.Ydd   = _accRated;
        trajStep.stage = TrapezoidalTrajectoryStage::kAccel;
    } else if (t < _accTime + _cruiseTime) {  // Coasting
        trajStep.Y     = _yAccel + _velRated * (t - _accTime);
        trajStep.Yd    = _velRated;
        trajStep.Ydd   = 0.0f;
        trajStep.stage = TrapezoidalTrajectoryStage::kCruise;
    } else if (t < _finalTime) {  // Deceleration
        f32 td         = t - _finalTime;
        trajStep.Y     = _endPos + 0.5f * _decRated * td * td;
        trajStep.Yd    = _decRated * td;
        trajStep.Ydd   = _decRated;
        trajStep.stage = TrapezoidalTrajectoryStage::kDecel;
    } else if (t >= _finalTime) {  // Final Condition
        trajStep.Y     = _endPos;
        trajStep.Yd    = 0.0f;
        trajStep.Ydd   = 0.0f;
        trajStep.stage = TrapezoidalTrajectoryStage::kFinal;
    } else {
        // Should not happen.
    }

    return trajStep;
}

TrapezoidalTrajectoryStep TrapezoidalTrajectory::evalByPos(f32 currentPos) {
    TrapezoidalTrajectoryStep trajStep;
    if (_startPos >= _endPos) {
        if (currentPos >= _startPos) {  // Initial Condition
            trajStep.Y     = _startPos;
            trajStep.Yd    = _startVel;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kInit;
        } else if (currentPos >= _yAccel) {  // Accelerating
            trajStep.Y     = currentPos;
            trajStep.Yd    = _startVel - std::sqrt(_accRated * (_startPos - currentPos) * 2.0f);
            trajStep.Ydd   = _accRated;
            trajStep.stage = TrapezoidalTrajectoryStage::kAccel;
        } else if (currentPos >= _yCruise) {  // Coasting
            trajStep.Y     = currentPos;
            trajStep.Yd    = _velRated;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kCruise;
        } else if (currentPos >= _endPos) {  // Deceleration
            trajStep.Y     = currentPos;
            trajStep.Yd    = std::sqrt(_accRated * (currentPos - _endPos) * 2.0f);
            trajStep.Ydd   = _decRated;
            trajStep.stage = TrapezoidalTrajectoryStage::kDecel;
        } else if (currentPos < _endPos) {  // Final Condition
            trajStep.Y     = _endPos;
            trajStep.Yd    = 0.0f;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kFinal;
        } else {
            // Should not happen.
        }
    } else {
        if (currentPos <= _startPos) {  // Initial Condition
            trajStep.Y     = _startPos;
            trajStep.Yd    = _startVel;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kInit;
        } else if (currentPos <= _yAccel) {  // Accelerating
            trajStep.Y     = currentPos;
            trajStep.Yd    = _startVel - std::sqrt(_accRated * (currentPos - _startPos) * 2.0f);
            trajStep.Ydd   = _accRated;
            trajStep.stage = TrapezoidalTrajectoryStage::kAccel;
        } else if (currentPos <= _yCruise) {  // Coasting
            trajStep.Y     = currentPos;
            trajStep.Yd    = _velRated;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kCruise;
        } else if (currentPos <= _endPos) {  // Deceleration
            trajStep.Y     = currentPos;
            trajStep.Yd    = std::sqrt(_accRated * (_endPos - currentPos) * 2.0f);
            trajStep.Ydd   = _decRated;
            trajStep.stage = TrapezoidalTrajectoryStage::kDecel;
        } else if (currentPos > _endPos) {  // Final Condition
            trajStep.Y     = _endPos;
            trajStep.Yd    = 0.0f;
            trajStep.Ydd   = 0.0f;
            trajStep.stage = TrapezoidalTrajectoryStage::kFinal;
        } else {
            // Should not happen.
        }
    }

    return trajStep;
}
void TrapezoidalTrajectory::setConfig(TrapezoidalTrajectoryConfig& config) {
    _config = config;
}

}  // namespace wibot
