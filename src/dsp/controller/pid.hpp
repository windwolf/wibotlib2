#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief PID 控制器（面向对象实现）
 * 
 * 将状态封装为成员，提供 process()/reset() 等接口。
 */
class Pid {
   public:
    enum class Mode {
        kSerial,    // 串行形式 (传统)
        kParallel,  // 并行形式
    };

    struct Config {
        Mode mode;                   // 控制器模式
        f32  Kp;                     // 比例增益
        f32  Ki;                     // 积分增益
        f32  Kd;                     // 微分增益
        f32  tau;                    // 微分低通滤波时间常数
        bool outputLimitEnable;      // 输出限制使能
        f32  outputLimitMax;         // 输出最大值
        f32  outputLimitMin;         // 输出最小值
        bool integratorLimitEnable;  // 积分限制使能
        f32  integratorLimitMax;     // 积分最大值
        f32  integratorLimitMin;     // 积分最小值
        f32  sampleTime;             // 采样时间（秒）
    };

    explicit Pid(const Config& cfg);

    /**
     * @brief 处理单个采样周期
     * @param measurement 测量值
     * @param setPoint    设定值
     * @return 控制输出
     */
    f32 update(f32 measurement, f32 setPoint);

    /**
     * @brief 重置状态
     */
    void reset();

    /**
     * @brief 重置积分项（防止积分饱和）
     */
    void resetIntegrator();

   private:
    const Config& _config;
    f32           _integrator;
    f32           _prevError;
    f32           _differentiator;
    f32           _prevMeasurement;
    f32           _output;
};

} // namespace wibot

