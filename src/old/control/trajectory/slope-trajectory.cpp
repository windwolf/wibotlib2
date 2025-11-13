//
// Created by zhouj on 2022/12/8.
//

#include "slope-trajectory.hpp"
#include <cmath>

namespace wibot {

bool SlopeTrajectory::plan(f32 startPos, f32 endPos, f32 vel) {
    f32 dX = endPos - startPos;       // Distance to travel
    _vel   = std::copysign(vel, dX);  // Maximum Velocity (signed)

    _finalTime = dX / _vel;
    _startPos  = startPos;
    _endPos    = endPos;
    return true;
}

SlopeTrajectoryStep SlopeTrajectory::evalByTime(f32 t) {
    SlopeTrajectoryStep trajStep;
    if (t < 0.0f) {  // Initial Condition
        trajStep.Y     = _startPos;
        trajStep.Yd    = 0.0f;
        trajStep.stage = SlopeTrajectoryStage::kInit;
    } else if (t < _finalTime) {  // slope
        f32 td         = _finalTime - t;
        trajStep.Y     = _endPos - _vel * td;
        trajStep.Yd    = _vel;
        trajStep.stage = SlopeTrajectoryStage::kCruise;
    } else if (t >= _finalTime) {  // Final Condition
        trajStep.Y     = _endPos;
        trajStep.Yd    = 0.0f;
        trajStep.stage = SlopeTrajectoryStage::kFinal;
    } else {
        // Should not happen.
    }

    return trajStep;
}
SlopeTrajectoryStep SlopeTrajectory::evelByPos(f32 currentPos) {
    SlopeTrajectoryStep trajStep;
    if (_startPos >= _endPos) {
        if (currentPos >= _startPos) {
            trajStep.Y     = _startPos;
            trajStep.Yd    = 0.0f;
            trajStep.stage = SlopeTrajectoryStage::kInit;
        } else if (currentPos >= _endPos) {
            trajStep.Y     = currentPos;
            trajStep.Yd    = _vel;
            trajStep.stage = SlopeTrajectoryStage::kCruise;
        } else if (currentPos < _endPos) {
            trajStep.Y     = _endPos;
            trajStep.Yd    = 0.0f;
            trajStep.stage = SlopeTrajectoryStage::kFinal;
        } else {
            // Should not happen.
        }
    } else {
        if (currentPos <= _startPos) {
            trajStep.Y     = _startPos;
            trajStep.Yd    = 0.0f;
            trajStep.stage = SlopeTrajectoryStage::kInit;
        } else if (currentPos <= _endPos) {
            trajStep.Y     = currentPos;
            trajStep.Yd    = _vel;
            trajStep.stage = SlopeTrajectoryStage::kCruise;
        } else if (currentPos > _endPos) {
            trajStep.Y     = _endPos;
            trajStep.Yd    = 0.0f;
            trajStep.stage = SlopeTrajectoryStage::kFinal;
        } else {
            // Should not happen.
        }
    }
    return trajStep;
}

}  // namespace wibot
