#pragma once

#include "arm_math.h"
#include "math.h"

namespace wibot {

class KalmanFilter {
   public:
    /**
     * @brief Construct a new Kalman filter object
     *
     * @param dimX state space dimension size
     * @param dimZ measurement space dimension size
     * @param F state transition matrix or jacobian matrix. matrix (x,x)
     * @param H measurement matrix or jacobian matrix. (z,x)
     */
    KalmanFilter(uint16_t dimX, uint16_t dimZ, arm_matrix_instance_f32& F,
                 arm_matrix_instance_f32& H)
        : _dimX(dimX), _dimZ(dimZ), _F(F), _H(H){};

    /**
     * @brief
     *
     * @param x initial state, column vector (x,1)
     * @param P covariance of state, matrix (x,x)
     */
    void init(arm_matrix_instance_f32* x, arm_matrix_instance_f32* P);

    /**
     * @brief
     *
     * @param u control inputs, already convert to state space, column vector (x,1)
     * @param Q covariance of process error, matrix (x,x)
     */
    void predict(const arm_matrix_instance_f32& u, const arm_matrix_instance_f32& Q);

    /**
     * @brief
     *
     * @param z observed values, column vector (z,1)
     * @param R covariance of observe error, matrix (z,z)
     */
    void update(const arm_matrix_instance_f32& z, const arm_matrix_instance_f32& R);

    void* userData;

   protected:
    /**
     * @brief state transition function. calculate the next state, sometimes update the F
     *
     * @param u control inputs, already convert to state space, column vector (N x 1)
     * @param Q covariance of process error, matrix (N x N)
     * @param x_next state of next step. column vector (N x 1)
     */
    virtual void transit(const arm_matrix_instance_f32& u, const arm_matrix_instance_f32& Q,
                         arm_matrix_instance_f32& x_next);
    /**
     * @brief
     *
     * @param z_estimate measurement estimated by estimate state. column vector (N x 1)
     */
    virtual void measure(arm_matrix_instance_f32& z_estimate);

    uint16_t                 _dimX;
    uint16_t                 _dimZ;
    arm_matrix_instance_f32* _x;  // state
    arm_matrix_instance_f32* _P;  // covariance of the state

    /**
     * @brief
     * For simple kf, out = F*_state+B*u, and update F if necessary.
     * For extended kf, estimate _state, out = f(_state, u, 0), and calculate jacobian matrix F_k <=
     * dF/dx|x_hat_k-1,u_k if necessary.
     */
    // KfFFunctonType F_func;

    /**
     * @brief
     * For simple kf, update the H if necessary.
     * For extended kf, estimate z_hat, out <= H(_state), and calculate jacobian matrix H_k <= dH/dx
     * | x_hat_k if necessary.
     */
    // KfHFunctonType H_func;

    arm_matrix_instance_f32& _F;
    arm_matrix_instance_f32& _H;
};

class ExtendedKalmanFilter : public KalmanFilter {
    using TransitionFunctionType = void (*)(const arm_matrix_instance_f32& x,
                                            const arm_matrix_instance_f32& u, void* userData,
                                            arm_matrix_instance_f32& x_next,
                                            arm_matrix_instance_f32& F);
    using MeasurementFunctonType = void (*)(const arm_matrix_instance_f32& x, void* userData,
                                            arm_matrix_instance_f32& z_estimate,
                                            arm_matrix_instance_f32& H);

   public:
    /**
     * @brief Construct a new Kalman filter object
     *
     * @param dimX state space dimension size
     * @param dimZ measurement space dimension size
     * @param F state transition matrix or jacobian matrix. matrix (x,x)
     * @param H measurement matrix or jacobian matrix. (z,x)
     * @param transitFunction state transition function, usually calculate next step state and
     * update the jacobian matrix F.
     * @param measureFunction measurement function, usually calculate estimate measurement and
     * update the jacobian matrix H.
     */
    ExtendedKalmanFilter(uint16_t dimX, uint16_t dimZ, arm_matrix_instance_f32& F,
                         arm_matrix_instance_f32& H, TransitionFunctionType transitFunction,
                         MeasurementFunctonType measureFunction)
        : KalmanFilter(dimX, dimZ, F, H),
          _F_function(transitFunction),
          _H_function(measureFunction){};

   protected:
    void transit(const arm_matrix_instance_f32& u, const arm_matrix_instance_f32& Q,
                 arm_matrix_instance_f32& x_next) override;
    void measure(arm_matrix_instance_f32& z_estimate) override;

   private:
    TransitionFunctionType _F_function;
    MeasurementFunctonType _H_function;
};

}  // namespace wibot
