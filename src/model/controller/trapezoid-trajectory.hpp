#pragma once

#include "model.hpp"
#include "type.hpp"
#include <algorithm>
#include <cmath>

namespace wibot {

/**
 * @brief 梯形加减速轨迹生成器配置参数
 */
struct TrapezoidTrajectoryConfig {
    f32 maxVelocity;   ///< 最大速度 (单位/秒)
    f32 acceleration;  ///< 加速度 (单位/秒²)
    f32 deceleration;  ///< 减速度 (单位/秒²)
    f32 sampleTime;    ///< 采样时间 (秒)

    /**
     * @brief 构造函数，设置默认参数
     */
    TrapezoidTrajectoryConfig()
        : maxVelocity(10.0f), acceleration(50.0f), deceleration(50.0f), sampleTime(0.01f) {
    }

    /**
     * @brief 构造函数
     * @param maxVel 最大速度 (单位/秒)
     * @param accel 加速度 (单位/秒²)
     * @param decel 减速度 (单位/秒²)
     * @param time 采样时间 (秒)
     */
    TrapezoidTrajectoryConfig(f32 maxVel, f32 accel, f32 decel, f32 time)
        : maxVelocity(maxVel), acceleration(accel), deceleration(decel), sampleTime(time) {
    }
};

/**
 * @brief 梯形轨迹状态
 */
enum class TrapezoidPhase : u8 {
    kIdle,          ///< 空闲状态
    kAcceleration,  ///< 加速段
    kConstant,      ///< 匀速段
    kDeceleration,  ///< 减速段
    kCompleted      ///< 完成状态
};

/**
 * @brief 梯形加减速曲线生成器管道
 * 
 * 实现SyncPipeline接口的梯形加减速轨迹生成器，从上游管道获取设定值，
 * 按照梯形曲线（加速-匀速-减速）从当前值逐渐变化到设定值。
 * 
 * 特性：
 * - 支持多通道并行处理
 * - 可配置最大速度、加速度和减速度
 * - 自动计算加速、匀速、减速三个阶段
 * - 支持正向和反向运动
 * 
 * @tparam CHANNELS 通道数量，必须大于0
 */
template <u8 CHANNELS = 1>
class TrapezoidTrajectory : public SyncPipeline<f32, f32*> {
   public:
    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供设定值
     */
    explicit TrapezoidTrajectory(SyncPipeline<f32, f32*>* upstream = nullptr);

    /**
     * @brief 更新管道状态
     */
    void update() override;

    /**
     * @brief 获取指定通道的输出值
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @return 当前输出值
     */
    f32 getValue(u8 channel) const override;

    /**
     * @brief 获取所有通道的输出值数组
     * @return 输出值数组指针
     */
    f32* getValues() const override;

    /**
     * @brief 重置管道状态
     */
    void reset() override;

    /**
     * @brief 设置配置参数
     * @param config 配置参数
     */
    void setConfig(const TrapezoidTrajectoryConfig& config);

    /**
     * @brief 获取当前配置参数
     * @return 当前配置参数
     */
    const TrapezoidTrajectoryConfig& getConfig() const;

    /**
     * @brief 设置上游管道
     * @param upstream 上游管道指针
     */
    void setUpstream(SyncPipeline<f32, f32*>* upstream);

    /**
     * @brief 设置指定通道的初始输出值
     * @param channel 通道索引
     * @param value 初始值
     */
    void setInitialValue(u8 channel, f32 value);

    /**
     * @brief 设置所有通道的初始输出值
     * @param values 初始值数组，长度必须为CHANNELS
     */
    void setInitialValues(const f32* values);

    /**
     * @brief 检查指定通道是否已到达设定值
     * @param channel 通道索引
     * @param tolerance 容差值，默认为1e-6
     * @return true 如果已到达设定值
     */
    bool isReached(u8 channel, f32 tolerance = 1e-6f) const;

    /**
     * @brief 检查所有通道是否都已到达设定值
     * @param tolerance 容差值，默认为1e-6
     * @return true 如果所有通道都已到达设定值
     */
    bool allReached(f32 tolerance = 1e-6f) const;

    /**
     * @brief 获取指定通道的当前运动阶段
     * @param channel 通道索引
     * @return 当前运动阶段
     */
    TrapezoidPhase getPhase(u8 channel) const;

    /**
     * @brief 获取指定通道的当前速度
     * @param channel 通道索引
     * @return 当前速度
     */
    f32 getVelocity(u8 channel) const;

   private:
    SyncPipeline<f32, f32*>*  _upstream;  ///< 上游管道指针
    TrapezoidTrajectoryConfig _config;    ///< 配置参数

    f32            _outputs[CHANNELS];     ///< 当前输出值数组
    f32            _setPoints[CHANNELS];   ///< 当前设定值数组
    f32            _velocities[CHANNELS];  ///< 当前速度数组
    TrapezoidPhase _phases[CHANNELS];      ///< 各通道运动阶段

    // 轨迹规划参数
    f32 _startPositions[CHANNELS];     ///< 起始位置
    f32 _targetPositions[CHANNELS];    ///< 目标位置
    f32 _accelDistances[CHANNELS];     ///< 加速段距离
    f32 _decelDistances[CHANNELS];     ///< 减速段距离
    f32 _constantDistances[CHANNELS];  ///< 匀速段距离
    f32 _accelTimes[CHANNELS];         ///< 加速时间
    f32 _constantTimes[CHANNELS];      ///< 匀速时间
    f32 _decelTimes[CHANNELS];         ///< 减速时间
    f32 _phaseTimers[CHANNELS];        ///< 各阶段计时器
    f32 _directions[CHANNELS];         ///< 运动方向 (+1 或 -1)

    /**
     * @brief 更新单个通道的输出值
     * @param channel 通道索引
     * @param setPoint 设定值
     */
    void updateChannel(u8 channel, f32 setPoint);

    /**
     * @brief 计算梯形轨迹参数
     * @param channel 通道索引
     * @param start 起始位置
     * @param target 目标位置
     */
    void calculateTrajectory(u8 channel, f32 start, f32 target);

    /**
     * @brief 限制数值在合理范围内
     * @param value 输入值
     * @return 限制后的值
     */
    f32 clampValue(f32 value) const;
};

// ============================================================================
// TrapezoidTrajectory 模板实现
// ============================================================================

template <u8 CHANNELS>
TrapezoidTrajectory<CHANNELS>::TrapezoidTrajectory(SyncPipeline<f32, f32*>* upstream)
    : _upstream(upstream), _config() {
    // 初始化所有通道的状态
    for (u8 i = 0; i < CHANNELS; ++i) {
        _outputs[i]           = 0.0f;
        _setPoints[i]         = 0.0f;
        _velocities[i]        = 0.0f;
        _phases[i]            = TrapezoidPhase::kIdle;
        _startPositions[i]    = 0.0f;
        _targetPositions[i]   = 0.0f;
        _accelDistances[i]    = 0.0f;
        _decelDistances[i]    = 0.0f;
        _constantDistances[i] = 0.0f;
        _accelTimes[i]        = 0.0f;
        _constantTimes[i]     = 0.0f;
        _decelTimes[i]        = 0.0f;
        _phaseTimers[i]       = 0.0f;
        _directions[i]        = 1.0f;
    }
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::update() {
    if (_upstream == nullptr) {
        // 没有上游管道，保持当前输出值不变
        return;
    }

    // 更新上游管道
    _upstream->update();

    // 处理所有通道
    for (u8 channel = 0; channel < CHANNELS; ++channel) {
        // 获取该通道的设定值
        f32 setPoint = _upstream->getValue(channel);
        updateChannel(channel, setPoint);
    }
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::updateChannel(u8 channel, f32 setPoint) {
    if (channel >= CHANNELS) {
        return;  // 超出范围，忽略
    }

    // 检查设定值是否发生变化
    if (std::abs(setPoint - _setPoints[channel]) > 1e-9f) {
        // 设定值改变，重新规划轨迹
        _setPoints[channel] = setPoint;
        calculateTrajectory(channel, _outputs[channel], setPoint);
    }

    // 根据当前阶段更新输出
    switch (_phases[channel]) {
        case TrapezoidPhase::kIdle:
        case TrapezoidPhase::kCompleted:
            // 空闲或完成状态，输出不变
            break;

        case TrapezoidPhase::kAcceleration: {
            // 加速段
            _phaseTimers[channel] += _config.sampleTime;

            if (_phaseTimers[channel] >= _accelTimes[channel]) {
                // 加速段结束，进入匀速段
                _phases[channel]      = TrapezoidPhase::kConstant;
                _phaseTimers[channel] = 0.0f;
                _velocities[channel]  = _config.maxVelocity * _directions[channel];
                _outputs[channel] =
                    _startPositions[channel] + _accelDistances[channel] * _directions[channel];
            } else {
                // 继续加速
                f32 t                = _phaseTimers[channel];
                _velocities[channel] = _config.acceleration * t * _directions[channel];
                _outputs[channel]    = _startPositions[channel] +
                                    0.5f * _config.acceleration * t * t * _directions[channel];
            }
            break;
        }

        case TrapezoidPhase::kConstant: {
            // 匀速段
            _phaseTimers[channel] += _config.sampleTime;

            if (_phaseTimers[channel] >= _constantTimes[channel]) {
                // 匀速段结束，进入减速段
                _phases[channel]      = TrapezoidPhase::kDeceleration;
                _phaseTimers[channel] = 0.0f;
                _outputs[channel] =
                    _startPositions[channel] +
                    (_accelDistances[channel] + _constantDistances[channel]) * _directions[channel];
            } else {
                // 继续匀速
                f32 t = _phaseTimers[channel];
                _outputs[channel] =
                    _startPositions[channel] +
                    (_accelDistances[channel] + _config.maxVelocity * t) * _directions[channel];
            }
            break;
        }

        case TrapezoidPhase::kDeceleration: {
            // 减速段
            _phaseTimers[channel] += _config.sampleTime;

            if (_phaseTimers[channel] >= _decelTimes[channel]) {
                // 减速段结束，到达目标
                _phases[channel]     = TrapezoidPhase::kCompleted;
                _velocities[channel] = 0.0f;
                _outputs[channel]    = _targetPositions[channel];
            } else {
                // 继续减速
                f32 t             = _phaseTimers[channel];
                f32 decelDistance = _config.maxVelocity * t - 0.5f * _config.deceleration * t * t;
                _velocities[channel] =
                    (_config.maxVelocity - _config.deceleration * t) * _directions[channel];
                _outputs[channel] =
                    _startPositions[channel] +
                    (_accelDistances[channel] + _constantDistances[channel] + decelDistance) *
                        _directions[channel];
            }
            break;
        }
    }

    // 限制输出值在合理范围内
    _outputs[channel] = clampValue(_outputs[channel]);
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::calculateTrajectory(u8 channel, f32 start, f32 target) {
    if (channel >= CHANNELS) {
        return;
    }

    _startPositions[channel]  = start;
    _targetPositions[channel] = target;

    // 计算总距离和方向
    f32 totalDistance    = target - start;
    _directions[channel] = (totalDistance >= 0) ? 1.0f : -1.0f;
    totalDistance        = std::abs(totalDistance);

    // 如果距离很小，直接完成
    if (totalDistance < 1e-6f) {
        _phases[channel]     = TrapezoidPhase::kCompleted;
        _velocities[channel] = 0.0f;
        _outputs[channel]    = target;
        return;
    }

    // 计算加速和减速所需的距离
    f32 accelDistance = (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.acceleration);
    f32 decelDistance = (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.deceleration);

    // 检查是否能达到最大速度（三角形轨迹 vs 梯形轨迹）
    if (accelDistance + decelDistance > totalDistance) {
        // 三角形轨迹：无法达到最大速度
        f32 peakVelocity =
            std::sqrt(2.0f * totalDistance * _config.acceleration * _config.deceleration /
                      (_config.acceleration + _config.deceleration));

        _accelDistances[channel]    = (peakVelocity * peakVelocity) / (2.0f * _config.acceleration);
        _decelDistances[channel]    = (peakVelocity * peakVelocity) / (2.0f * _config.deceleration);
        _constantDistances[channel] = 0.0f;

        _accelTimes[channel]    = peakVelocity / _config.acceleration;
        _constantTimes[channel] = 0.0f;
        _decelTimes[channel]    = peakVelocity / _config.deceleration;
    } else {
        // 梯形轨迹：能达到最大速度
        _accelDistances[channel]    = accelDistance;
        _decelDistances[channel]    = decelDistance;
        _constantDistances[channel] = totalDistance - accelDistance - decelDistance;

        _accelTimes[channel]    = _config.maxVelocity / _config.acceleration;
        _constantTimes[channel] = _constantDistances[channel] / _config.maxVelocity;
        _decelTimes[channel]    = _config.maxVelocity / _config.deceleration;
    }

    // 开始轨迹执行
    _phases[channel]      = TrapezoidPhase::kAcceleration;
    _phaseTimers[channel] = 0.0f;
    _velocities[channel]  = 0.0f;
}

template <u8 CHANNELS>
f32 TrapezoidTrajectory<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 超出范围返回0
    }
    return _outputs[channel];
}

template <u8 CHANNELS>
f32* TrapezoidTrajectory<CHANNELS>::getValues() const {
    return const_cast<f32*>(_outputs);
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::reset() {
    for (u8 i = 0; i < CHANNELS; ++i) {
        _outputs[i]           = 0.0f;
        _setPoints[i]         = 0.0f;
        _velocities[i]        = 0.0f;
        _phases[i]            = TrapezoidPhase::kIdle;
        _startPositions[i]    = 0.0f;
        _targetPositions[i]   = 0.0f;
        _accelDistances[i]    = 0.0f;
        _decelDistances[i]    = 0.0f;
        _constantDistances[i] = 0.0f;
        _accelTimes[i]        = 0.0f;
        _constantTimes[i]     = 0.0f;
        _decelTimes[i]        = 0.0f;
        _phaseTimers[i]       = 0.0f;
        _directions[i]        = 1.0f;
    }
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::setConfig(const TrapezoidTrajectoryConfig& config) {
    _config = config;

    // 确保参数合理性
    if (_config.maxVelocity <= 0) {
        _config.maxVelocity = 10.0f;
    }

    if (_config.acceleration <= 0) {
        _config.acceleration = 50.0f;
    }

    if (_config.deceleration <= 0) {
        _config.deceleration = 50.0f;
    }

    if (_config.sampleTime <= 0) {
        _config.sampleTime = 0.01f;
    }
}

template <u8 CHANNELS>
const TrapezoidTrajectoryConfig& TrapezoidTrajectory<CHANNELS>::getConfig() const {
    return _config;
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::setUpstream(SyncPipeline<f32, f32*>* upstream) {
    _upstream = upstream;
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::setInitialValue(u8 channel, f32 value) {
    if (channel < CHANNELS) {
        _outputs[channel]    = clampValue(value);
        _setPoints[channel]  = _outputs[channel];
        _phases[channel]     = TrapezoidPhase::kIdle;
        _velocities[channel] = 0.0f;
    }
}

template <u8 CHANNELS>
void TrapezoidTrajectory<CHANNELS>::setInitialValues(const f32* values) {
    if (values == nullptr) {
        return;
    }

    for (u8 i = 0; i < CHANNELS; ++i) {
        setInitialValue(i, values[i]);
    }
}

template <u8 CHANNELS>
bool TrapezoidTrajectory<CHANNELS>::isReached(u8 channel, f32 tolerance) const {
    if (channel >= CHANNELS) {
        return false;
    }

    return _phases[channel] == TrapezoidPhase::kCompleted &&
           std::abs(_outputs[channel] - _setPoints[channel]) <= tolerance;
}

template <u8 CHANNELS>
bool TrapezoidTrajectory<CHANNELS>::allReached(f32 tolerance) const {
    for (u8 i = 0; i < CHANNELS; ++i) {
        if (!isReached(i, tolerance)) {
            return false;
        }
    }
    return true;
}

template <u8 CHANNELS>
TrapezoidPhase TrapezoidTrajectory<CHANNELS>::getPhase(u8 channel) const {
    if (channel >= CHANNELS) {
        return TrapezoidPhase::kIdle;
    }
    return _phases[channel];
}

template <u8 CHANNELS>
f32 TrapezoidTrajectory<CHANNELS>::getVelocity(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;
    }
    return _velocities[channel];
}

template <u8 CHANNELS>
f32 TrapezoidTrajectory<CHANNELS>::clampValue(f32 value) const {
    // 检查是否为有效数值
    if (std::isnan(value) || std::isinf(value)) {
        return 0.0f;
    }

    // 限制在合理范围内，防止数值溢出
    const f32 maxValue = 1e6f;
    const f32 minValue = -1e6f;

    return std::max(minValue, std::min(maxValue, value));
}

}  // namespace wibot