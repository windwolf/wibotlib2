#pragma once

//
// Created by zhouj on 2022/12/8.
//

#include "type.hpp"
namespace wibot {
struct TrapezoidalTrajectoryConfig {
    f32 velLimit   = 2.0f;  // [turn/s]
    f32 accelLimit = 0.5f;  // [turn/s^2]
    f32 decelLimit = 0.5f;  // [turn/s^2]
};

enum class TrapezoidalTrajectoryStage {
    kInit,
    kAccel,
    kCruise,
    kDecel,
    kFinal,
};

struct TrapezoidalTrajectoryStep {
    f32                        Y;
    f32                        Yd;
    f32                        Ydd;
    TrapezoidalTrajectoryStage stage;
};

class TrapezoidalTrajectory {
   public:
    void setConfig(TrapezoidalTrajectoryConfig& config);

    bool plan(f32 start_pos, f32 end_pos, f32 start_vel, f32 max_vel, f32 max_acc, f32 max_dec);
    /**
     * Eval step info by current time tick.
     * @note If use this function for pos control, succed
     * If use this function for vel (not pos) control, it become a open loop control,
     * so it will lose sync when real pos is not equal to the pos calculated by this function.
     *
     * @param t
     * @return
     */
    TrapezoidalTrajectoryStep evalByTime(f32 t);

    /**
     * Eval step info by current position.
     *
     * @param currentPos
     * @return
     */
    TrapezoidalTrajectoryStep evalByPos(f32 currentPos);

   private:
    TrapezoidalTrajectoryConfig _config;

    f32 _startPos;
    f32 _endPos;
    f32 _startVel;

    f32 _accRated;
    f32 _velRated;
    f32 _decRated;

    f32 _accTime;
    f32 _cruiseTime;
    f32 _decTime;
    f32 _finalTime;

    f32 _yAccel;
    f32 _yCruise;
};

}  // namespace wibot
