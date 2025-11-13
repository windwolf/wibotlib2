#include "pid.hpp"

namespace wibot {

void PidController::reset() {
    /* Clear controller variables */
    _integrator = 0.0f;
    _prevError  = 0.0f;

    _differentiator  = 0.0f;
    _prevMeasurement = 0.0f;
};

f32 PidController::update(f32 setPoint, f32 measurement) {
    switch (config.mode) {
        case PidControllerMode::kSerial:
            return updateSerial(setPoint, measurement);
        case PidControllerMode::kParallel:
            return updateParallel(setPoint, measurement);
        default:
            return 0.0f;
    }
};

/**
 * @brief y = Kp + Ki/s + Kd*s/(s*tua+1)
 * p_k = Kp*e_k;
 * i_k = Ki*T/2*(e_k+e_k1) + i_k1;
 * d_k = 2*Kd/(2*tau+T)*(e_k-e_k1) + (2*tau-T)/(2*tau+T)*d_k1;
 * o_k = p_k + i_k + d_k;
 * @param setPoint
 * @param measurement
 * @return f32
 */
f32 PidController::updateParallel(f32 setPoint, f32 measurement) {
    f32 out;

    /*
     * Error signal
     */
    f32 error = setPoint - measurement;

    /*
     * Proportional
     */
    f32 proportional = config.Kp * error;

    /*
     * Integral
     */
    _integrator = _integrator + 0.5f * config.Ki * config.sampleTime * (error + _prevError);

    /* Anti-wind-up via _integrator clamping */
    if (config.integratorLimitEnable) {
        if (_integrator > config.integratorLimitMax) {
            _integrator = config.integratorLimitMax;
        } else if (_integrator < config.integratorLimitMin) {
            _integrator = config.integratorLimitMin;
        }
    }

    /*
     * Derivative (band-limited _differentiator)
     */

    _differentiator =
        -(2.0f * config.Kd *
              (measurement - _prevMeasurement) /* Note: derivative on measurement, therefore minus
                                                  sign in front of equation! */
          + (2.0f * config.tau - config.sampleTime) * _differentiator) /
        (2.0f * config.tau + config.sampleTime);

    /*
     * Compute output and apply limits
     */
    out = proportional + _integrator + _differentiator;

    if (config.outputLimitEnable) {
        if (out > config.outputLimitMax) {
            out = config.outputLimitMax;
        } else if (out < config.outputLimitMin) {
            out = config.outputLimitMin;
        }
    }

    /* Store error and measurement for later use */
    _prevError       = error;
    _prevMeasurement = measurement;

    /* Return controller output */
    return out;
}

/**
 * @brief y = Kp(1 + Ki/s + Kd*s/(s*tua+1))
 * p_k = e_k;
 * i_k = Ki*T/2*(e_k+e_k1) + i_k1;
 * d_k = 2*Kd/(2*tau+T)*(e_k-e_k1) + (2*tau-T)/(2*tau+T)*d_k1;
 * o_k = Kp*(p_k + i_k + d_k);
 * @param setpointcontext.
 * @param measurement
 * @return f32
 */
f32 PidController::updateSerial(f32 setPoint, f32 measurement) {
    f32 out;

    /*
     * Error signal
     */
    f32 error = setPoint - measurement;

    /*
     * Proportional
     */
    f32 proportional = error;

    /*
     * Integral
     */
    _integrator = _integrator + 0.5f * config.Ki * config.sampleTime * (error + _prevError);

    if (config.integratorLimitEnable) {
        /* Anti-wind-up via _integrator clamping */
        if (_integrator > config.integratorLimitMax) {
            _integrator = config.integratorLimitMax;
        } else if (_integrator < config.integratorLimitMin) {
            _integrator = config.integratorLimitMin;
        }
    }

    /*
     * Derivative (band-limited _differentiator)
     */

    _differentiator =
        -(2.0f * config.Kd * (measurement - _prevMeasurement)
          /* Note: derivative on measurement, therefore minus sign in front of equation! */
          + (2.0f * config.tau - config.sampleTime) * _differentiator) /
        (2.0f * config.tau + config.sampleTime);

    /*
     * Compute output and apply limits
     */
    out = config.Kp * (proportional + _integrator + _differentiator);

    if (config.outputLimitEnable) {
        if (out > config.outputLimitMax) {
            out = config.outputLimitMax;
        } else if (out < config.outputLimitMin) {
            out = config.outputLimitMin;
        }
    }

    /* Store error and measurement for later use */
    _prevError       = error;
    _prevMeasurement = measurement;

    /* Return controller output */
    return out;
}
PidController::PidController() {
    reset();
}

}  // namespace wibot
