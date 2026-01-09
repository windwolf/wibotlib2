#pragma once

#include "type.hpp"

namespace wibot::dsp {

class TrapezoidTrajectory {
   public:
    struct Config {
        f32 maxVelocity{10.0f};   // 最大速度 (单位/秒)
        f32 acceleration{50.0f};  // 加速度 (单位/秒²)
        f32 deceleration{50.0f};  // 减速度 (单位/秒²)
        f32 sampleTime{0.01f};    // 采样时间 (秒)
    };
    enum class Phase : u8 {
        kIdle,
        kAcceleration,
        kConstant,
        kDeceleration,
        kCompleted
    };
    struct State {
        f32   output{0.0f};
        f32   setPoint{0.0f};
        f32   velocity{0.0f};
        Phase phase{Phase::kIdle};
        f32   startPosition{0.0f};
        f32   targetPosition{0.0f};
        f32   accelDistance{0.0f};
        f32   decelDistance{0.0f};
        f32   constantDistance{0.0f};
        f32   accelTime{0.0f};
        f32   constantTime{0.0f};
        f32   decelTime{0.0f};
        f32   phaseTimer{0.0f};
        f32   direction{1.0f};
    };

   public:
    explicit TrapezoidTrajectory(Config& config);

    void reset();

    void setInitialValue(f32 value);

    f32 update(f32 setPoint);

    f32   getVelocity() const;
    Phase getPhase() const;

    bool isReached(f32 tolerance = 1e-6f) const;

   private:
    void accelStep();

    void constantStep();

    void decelStep();

    void calculateTrajectory(f32 start, f32 target);

    f32 clampValue(f32 value) const;

   private:
    Config& _config;
    State   _state{};
};

}  // namespace wibot::dsp
