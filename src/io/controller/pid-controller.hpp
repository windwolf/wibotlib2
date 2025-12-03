#pragma once

#include "model.hpp"
#include "type.hpp"
#include <algorithm>

namespace wibot {

/**
 * @brief PID控制器模式
 */
enum class PidControllerMode {
    kSerial,    ///< 串行形式 (传统形式)
    kParallel,  ///< 并行形式
};

/**
 * @brief PID控制器配置参数
 */
struct PidControllerConfig {
    PidControllerMode mode;  ///< 控制器模式

    /* 控制器增益 */
    f32 Kp;  ///< 比例增益
    f32 Ki;  ///< 积分增益
    f32 Kd;  ///< 微分增益

    /* 微分低通滤波器时间常数 */
    f32 tau;  ///< 微分项滤波时间常数

    /* 输出限制 */
    bool outputLimitEnable;  ///< 输出限制使能
    f32  outputLimitMax;     ///< 输出最大值
    f32  outputLimitMin;     ///< 输出最小值

    /* 积分限制 */
    bool integratorLimitEnable;  ///< 积分限制使能
    f32  integratorLimitMax;     ///< 积分最大值
    f32  integratorLimitMin;     ///< 积分最小值

    /* 采样时间 (秒) */
    f32 sampleTime;  ///< 采样时间

    /* 设定值 */
    f32 setPoint;  ///< 目标设定值
};

/**
 * @brief PID控制器管道
 * 
 * 实现SyncPipeline接口的PID控制器，从上游管道获取测量值，
 * 输出控制量。支持串行和并行两种PID形式。
 * 
 * 配置使用引用方式，支持多个控制器实例共享同一配置。
 *
 * 管道流程：
 * 上游测量值 -> PID控制器 -> 控制输出
 */
class PidController : public SyncPipeline<f32> {
   public:
    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供测量值
     * @param config PID配置参数（引用方式，支持共享）
     */
    explicit PidController(SyncPipeline<f32>& upstream, const PidControllerConfig& config)
        : _upstream(upstream),
          _config(config),
          _integrator(0.0f),
          _prevError(0.0f),
          _differentiator(0.0f),
          _prevMeasurement(0.0f),
          _output(0.0f) {
    }

    /**
     * @brief 构造函数(使用默认配置)
     */
    explicit PidController(SyncPipeline<f32>& upstream)
        : _upstream(upstream),
          _config(_defaultConfig),
          _integrator(0.0f),
          _prevError(0.0f),
          _differentiator(0.0f),
          _prevMeasurement(0.0f),
          _output(0.0f) {
    }

    /**
     * @brief 析构函数
     */
    virtual ~PidController() = default;

    /**
     * @brief 更新管道状态
     * 
     * 从上游获取测量值，计算PID控制输出
     */
    void update() override {
        _upstream.update();

        // 获取测量值
        f32 measurement = _upstream.getValue();

        // 计算误差
        f32 error = _config.setPoint - measurement;

        // 根据模式计算PID输出
        f32 pidOutput;
        if (_config.mode == PidControllerMode::kSerial) {
            pidOutput = updateSerial(error, measurement);
        } else {
            pidOutput = updateParallel(error, measurement);
        }

        // 应用输出限制
        _output = applyOutputLimit(pidOutput);
    }

    f32 getValue() const override {
        return _output;
    }

    void reset() override {
        _integrator      = 0.0f;
        _prevError       = 0.0f;
        _differentiator  = 0.0f;
        _prevMeasurement = 0.0f;
        _output          = 0.0f;
    }

    /**
     * @brief 获取PID参数配置
     */
    const PidControllerConfig& getConfig() const {
        return _config;
    }

   private:
    /**
     * @brief 串行PID更新计算
     */
    f32 updateSerial(f32 error, f32 measurement) {
        // 比例项
        f32 proportional = _config.Kp * error;

        // 积分项
        _integrator += 0.5f * _config.Ki * _config.sampleTime * (error + _prevError);
        _integrator = applyIntegratorLimit(_integrator);

        // 微分项 (使用测量值微分，避免设定值突变影响)
        _differentiator = -(2.0f * _config.Kd * (measurement - _prevMeasurement) +
                            (2.0f * _config.tau - _config.sampleTime) * _differentiator) /
                          (2.0f * _config.tau + _config.sampleTime);
        _prevMeasurement = measurement;

        // 保存当前误差用于下次积分计算
        _prevError = error;

        return proportional + _integrator + _differentiator;
    }

    /**
     * @brief 并行PID更新计算
     */
    f32 updateParallel(f32 error, f32 measurement) {
        // 比例项
        f32 proportional = _config.Kp * error;

        // 积分项
        _integrator += 0.5f * _config.Ki * _config.sampleTime * (error + _prevError);
        _integrator = applyIntegratorLimit(_integrator);

        // 微分项
        _differentiator = -(2.0f * _config.Kd * (measurement - _prevMeasurement) +
                            (2.0f * _config.tau - _config.sampleTime) * _differentiator) /
                          (2.0f * _config.tau + _config.sampleTime);
        _prevMeasurement = measurement;

        _prevError = error;

        return proportional + _integrator + _differentiator;
    }

    /**
     * @brief 应用输出限制
     * @param output 原始输出值
     * @return f32 限制后的输出值
     */
    f32 applyOutputLimit(f32 output) {
        if (!_config.outputLimitEnable) {
            return output;
        }

        return std::max(_config.outputLimitMin, std::min(_config.outputLimitMax, output));
    }

    /**
     * @brief 应用积分限制
     * @param integrator 原始积分值
     * @return f32 限制后的积分值
     */
    f32 applyIntegratorLimit(f32 integrator) {
        if (!_config.integratorLimitEnable) {
            return integrator;
        }

        return std::max(_config.integratorLimitMin,
                        std::min(_config.integratorLimitMax, integrator));
    }

   private:
    SyncPipeline<f32>&         _upstream;  ///< 上游管道引用
    const PidControllerConfig& _config;    ///< PID配置参数引用（支持共享）

    /* 控制器内部状态 */
    f32 _integrator;       ///< 积分项
    f32 _prevError;        ///< 上一次误差
    f32 _differentiator;   ///< 微分项
    f32 _prevMeasurement;  ///< 上一次测量值

    /* 输出缓存 */
    f32 _output;  ///< 当前输出值

    /* 默认配置 */
    static inline const PidControllerConfig _defaultConfig = {.mode = PidControllerMode::kSerial,
                                                              .Kp   = 1.0f,
                                                              .Ki   = 0.0f,
                                                              .Kd   = 0.0f,
                                                              .tau  = 0.0f,
                                                              .outputLimitEnable     = false,
                                                              .outputLimitMax        = 100.0f,
                                                              .outputLimitMin        = -100.0f,
                                                              .integratorLimitEnable = false,
                                                              .integratorLimitMax    = 100.0f,
                                                              .integratorLimitMin    = -100.0f,
                                                              .sampleTime            = 0.01f,
                                                              .setPoint              = 0.0f};
};

}  // namespace wibot