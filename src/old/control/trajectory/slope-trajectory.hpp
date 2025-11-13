#pragma once

//
// Created by zhouj on 2022/12/8.
//

namespace wibot {

enum class SlopeTrajectoryStage {
    kInit,
    kCruise,
    kFinal,
};

struct SlopeTrajectoryStep {
    f32                  Y;
    f32                  Yd;
    SlopeTrajectoryStage stage;
};

class SlopeTrajectory {
   public:
    bool                plan(f32 startPos, f32 endPos, f32 vel);
    SlopeTrajectoryStep evalByTime(f32 t);
    SlopeTrajectoryStep evelByPos(f32 currentPos);

   private:
    f32 _startPos;
    f32 _endPos;

    f32 _vel;

    f32 _finalTime;
};

}  // namespace wibot
