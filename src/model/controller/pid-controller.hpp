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
 * 管道流程：
 * 上游测量值 -> PID控制器 -> 控制输出
 */
template <u8 CHANNELS = 1>
class PidController : public SyncPipeline<f32> {
   public:
    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供测量值
     */
    explicit PidController(SyncPipeline<f32>* upstream);

    /**
     * @brief 析构函数
     */
    virtual ~PidController() = default;

    /**
     * @brief 更新管道状态
     * 
     * 从上游获取测量值，计算PID控制输出
     */
    void update() override;

    /**
     * @brief 获取控制输出值
     * @param channel 通道索引
     * @return f32 控制输出值
     */
    f32 getValue(u8 channel = 0) const override;

    /**
     * @brief 获取所有通道的输出值
     * @return f32* 所有通道的输出值
     */
    f32* getValues() const override;

    /**
     * @brief 重置控制器状态
     * 
     * 清除积分项、微分项等内部状态
     */
    void reset() override;

    /**
     * @brief 设置PID参数配置（所有通道共享）
     * @param config PID配置参数
     */
    void setConfig(const PidControllerConfig& config);

    /**
     * @brief 获取PID参数配置
     * @return const PidControllerConfig& PID配置参数
     */
    const PidControllerConfig& getConfig() const;

    /**
     * @brief 设置目标设定值
     * @param setPoint 目标值（所有通道共享）
     */
    void setSetPoint(f32 setPoint);

    /**
     * @brief 获取目标设定值
     * @return f32 目标值
     */
    f32 getSetPoint() const;

   private:
    /**
     * @brief 串行PID更新计算
     * @param error 误差值
     * @param channel 通道索引
     * @return f32 控制输出
     */
    f32 updateSerial(f32 error, u8 channel);

    /**
     * @brief 并行PID更新计算
     * @param error 误差值
     * @param channel 通道索引
     * @return f32 控制输出
     */
    f32 updateParallel(f32 error, u8 channel);

    /**
     * @brief 应用输出限制
     * @param output 原始输出值
     * @return f32 限制后的输出值
     */
    f32 applyOutputLimit(f32 output);

    /**
     * @brief 应用积分限制
     * @param integrator 原始积分值
     * @return f32 限制后的积分值
     */
    f32 applyIntegratorLimit(f32 integrator);

   private:
    SyncPipeline<f32>*  _upstream;  ///< 上游管道指针
    PidControllerConfig _config;    ///< PID配置参数（所有通道共享）

    /* 控制器内部状态 */
    f32 _integrator[CHANNELS];       ///< 积分项
    f32 _prevError[CHANNELS];        ///< 上一次误差 (用于积分计算)
    f32 _differentiator[CHANNELS];   ///< 微分项
    f32 _prevMeasurement[CHANNELS];  ///< 上一次测量值 (用于微分计算)

    /* 输出缓存 */
    // f32  _output;             ///< 当前输出值
    f32 _outputs[CHANNELS];  ///< 所有通道输出值 (当需要多通道时)
};

// ============================================================================
// PidController 模板实现
// ============================================================================

template <u8 CHANNELS>
PidController<CHANNELS>::PidController(SyncPipeline<f32>* upstream)
    : _upstream(upstream), _config{} {
    // 初始化所有通道的状态
    for (u8 i = 0; i < CHANNELS; ++i) {
        _integrator[i]      = 0.0f;
        _prevError[i]       = 0.0f;
        _differentiator[i]  = 0.0f;
        _prevMeasurement[i] = 0.0f;
        _outputs[i]         = 0.0f;
    }

    // 设置默认配置（所有通道共享）
    _config.mode                  = PidControllerMode::kSerial;
    _config.Kp                    = 1.0f;
    _config.Ki                    = 0.0f;
    _config.Kd                    = 0.0f;
    _config.tau                   = 0.0f;
    _config.outputLimitEnable     = false;
    _config.outputLimitMax        = 100.0f;
    _config.outputLimitMin        = -100.0f;
    _config.integratorLimitEnable = false;
    _config.integratorLimitMax    = 100.0f;
    _config.integratorLimitMin    = -100.0f;
    _config.sampleTime            = 0.01f;  // 10ms默认采样时间
    _config.setPoint              = 0.0f;
}

template <u8 CHANNELS>
void PidController<CHANNELS>::update() {
    if (_upstream == nullptr) {
        // 没有上游管道，所有通道输出0
        for (u8 i = 0; i < CHANNELS; ++i) {
            _outputs[i] = 0.0f;
        }
        return;
    }

    // 更新上游管道
    _upstream->update();

    // 处理所有通道
    for (u8 channel = 0; channel < CHANNELS; ++channel) {
        // 获取该通道的测量值
        f32 measurement = _upstream->getValue(channel);

        // 计算误差
        f32 error = _config.setPoint - measurement;

        // 根据模式计算PID输出
        f32 pidOutput;
        if (_config.mode == PidControllerMode::kSerial) {
            pidOutput = updateSerial(error, channel);
        } else {
            pidOutput = updateParallel(error, channel);
        }

        // 应用输出限制
        pidOutput = applyOutputLimit(pidOutput);

        // 保存输出
        _outputs[channel] = pidOutput;
    }
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 超出范围返回0
    }
    return _outputs[channel];
}

template <u8 CHANNELS>
f32* PidController<CHANNELS>::getValues() const {
    return const_cast<f32*>(_outputs);
}

template <u8 CHANNELS>
void PidController<CHANNELS>::reset() {
    for (u8 i = 0; i < CHANNELS; ++i) {
        _integrator[i]      = 0.0f;
        _prevError[i]       = 0.0f;
        _differentiator[i]  = 0.0f;
        _prevMeasurement[i] = 0.0f;
        _outputs[i]         = 0.0f;
    }
}

template <u8 CHANNELS>
void PidController<CHANNELS>::setConfig(const PidControllerConfig& config) {
    _config = config;
}

template <u8 CHANNELS>
const PidControllerConfig& PidController<CHANNELS>::getConfig() const {
    return _config;
}

template <u8 CHANNELS>
void PidController<CHANNELS>::setSetPoint(f32 setPoint) {
    _config.setPoint = setPoint;
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::getSetPoint() const {
    return _config.setPoint;
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::updateSerial(f32 error, u8 channel) {
    // 串行PID形式: u(k) = Kp * (e(k) + (1/Ti) * ∫e(t)dt + Td * de(t)/dt)

    // 比例项
    f32 proportional = _config.Kp * error;

    // 积分项
    _integrator[channel] += 0.5f * _config.Ki * _config.sampleTime * (error + _prevError[channel]);
    _integrator[channel] = applyIntegratorLimit(_integrator[channel]);

    // 微分项 (使用测量值微分，避免设定值突变影响)
    if (_upstream != nullptr) {
        f32 measurement = _upstream->getValue(channel);
        _differentiator[channel] =
            -(2.0f * _config.Kd * (measurement - _prevMeasurement[channel]) +
              (2.0f * _config.tau - _config.sampleTime) * _differentiator[channel]) /
            (2.0f * _config.tau + _config.sampleTime);
        _prevMeasurement[channel] = measurement;
    }

    // 保存当前误差用于下次积分计算
    _prevError[channel] = error;

    // 计算总输出
    return proportional + _integrator[channel] + _differentiator[channel];
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::updateParallel(f32 error, u8 channel) {
    // 并行PID形式: u(k) = Kp * e(k) + Ki * ∫e(t)dt + Kd * de(t)/dt

    // 比例项
    f32 proportional = _config.Kp * error;

    // 积分项
    _integrator[channel] += 0.5f * _config.Ki * _config.sampleTime * (error + _prevError[channel]);
    _integrator[channel] = applyIntegratorLimit(_integrator[channel]);

    // 微分项 (使用测量值微分，避免设定值突变影响)
    if (_upstream != nullptr) {
        f32 measurement = _upstream->getValue(channel);
        _differentiator[channel] =
            -(2.0f * _config.Kd * (measurement - _prevMeasurement[channel]) +
              (2.0f * _config.tau - _config.sampleTime) * _differentiator[channel]) /
            (2.0f * _config.tau + _config.sampleTime);
        _prevMeasurement[channel] = measurement;
    }

    // 保存当前误差用于下次积分计算
    _prevError[channel] = error;

    // 计算总输出
    return proportional + _integrator[channel] + _differentiator[channel];
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::applyOutputLimit(f32 output) {
    if (!_config.outputLimitEnable) {
        return output;
    }

    return std::max(_config.outputLimitMin, std::min(_config.outputLimitMax, output));
}

template <u8 CHANNELS>
f32 PidController<CHANNELS>::applyIntegratorLimit(f32 integrator) {
    if (!_config.integratorLimitEnable) {
        return integrator;
    }

    return std::max(_config.integratorLimitMin, std::min(_config.integratorLimitMax, integrator));
}

}  // namespace wibot