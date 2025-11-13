#pragma once

#include "type.hpp"

namespace wibot {
enum class PidControllerMode {
    kSerial,
    kParallel,
};
struct PidControllerConfig {
    PidControllerMode mode;
    /* Controller gains */
    f32               Kp;
    f32               Ki;
    f32               Kd;

    /* Derivative low-pass filter time constant */
    f32 tau;

    /* Output limits */
    bool outputLimitEnable;
    f32  outputLimitMax;
    f32  outputLimitMin;

    /* Integrator limits */
    bool integratorLimitEnable;
    f32  integratorLimitMax;
    f32  integratorLimitMin;

    /* Sample time (in seconds) */
    f32 sampleTime;
};

class PidController {
   public:
    PidController();
    void reset();
    f32  update(f32 setPoint, f32 measurement);

   protected:
   private:
    f32 updateSerial(f32 setPoint, f32 measurement);
    f32 updateParallel(f32 setPoint, f32 measurement);

   public:
    PidControllerConfig config;

   private:
    /* Controller "memory" */
    f32 _integrator;
    f32 _prevError; /* Required for integrator_ */
    f32 _differentiator;
    f32 _prevMeasurement; /* Required for differentiator_ */
};

}  // namespace wibot
