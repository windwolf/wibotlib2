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
 * - 可配置最大速度、加速度和减速度
 * - 自动计算加速、匀速、减速三个阶段
 * - 支持正向和反向运动
 */
class TrapezoidTrajectory : public SyncPipeline<f32> {
   public:
    struct Storage {
        f32            output{0.0f};
        f32            setPoint{0.0f};
        f32            velocity{0.0f};
        TrapezoidPhase phase{TrapezoidPhase::kIdle};
        f32            startPosition{0.0f};
        f32            targetPosition{0.0f};
        f32            accelDistance{0.0f};
        f32            decelDistance{0.0f};
        f32            constantDistance{0.0f};
        f32            accelTime{0.0f};
        f32            constantTime{0.0f};
        f32            decelTime{0.0f};
        f32            phaseTimer{0.0f};
        f32            direction{1.0f};
    };

    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供设定值
     */
    explicit TrapezoidTrajectory(SyncPipeline<f32>& upstream, Storage& storage)
        : _upstream(upstream), _config(), _storage(storage) {
        reset();
    }

    /**
     * @brief 更新管道状态
     */
    void update() override {
        _upstream.update();
        f32 setPoint = _upstream.getValue();
        updateTrajectory(setPoint);
    }

    /**
     * @brief 获取当前输出值
     * @return 当前输出值
     */
    f32 getValue() const override {
        return _storage.output;
    }

    /**
     * @brief 重置管道状态
     */
    void reset() override {
        _storage.output           = 0.0f;
        _storage.setPoint         = 0.0f;
        _storage.velocity         = 0.0f;
        _storage.phase            = TrapezoidPhase::kIdle;
        _storage.startPosition    = 0.0f;
        _storage.targetPosition   = 0.0f;
        _storage.accelDistance    = 0.0f;
        _storage.decelDistance    = 0.0f;
        _storage.constantDistance = 0.0f;
        _storage.accelTime        = 0.0f;
        _storage.constantTime     = 0.0f;
        _storage.decelTime        = 0.0f;
        _storage.phaseTimer       = 0.0f;
        _storage.direction        = 1.0f;
    }

    /**
     * @brief 设置配置参数
     * @param config 配置参数
     */
    void setConfig(const TrapezoidTrajectoryConfig& config) {
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

    /**
     * @brief 获取当前配置参数
     * @return 当前配置参数
     */
    const TrapezoidTrajectoryConfig& getConfig() const {
        return _config;
    }

    /**
     * @brief 设置初始输出值
     * @param value 初始值
     */
    void setInitialValue(f32 value) {
        _storage.output   = clampValue(value);
        _storage.setPoint = _storage.output;
        _storage.phase    = TrapezoidPhase::kIdle;
        _storage.velocity = 0.0f;
    }

    /**
     * @brief 检查是否已到达设定值
     * @param tolerance 容差值，默认为1e-6
     * @return true 如果已到达设定值
     */
    bool isReached(f32 tolerance = 1e-6f) const {
        return _storage.phase == TrapezoidPhase::kCompleted &&
               std::abs(_storage.output - _storage.setPoint) <= tolerance;
    }

    /**
     * @brief 获取当前运动阶段
     * @return 当前运动阶段
     */
    TrapezoidPhase getPhase() const {
        return _storage.phase;
    }

    /**
     * @brief 获取当前速度
     * @return 当前速度
     */
    f32 getVelocity() const {
        return _storage.velocity;
    }

   private:
    SyncPipeline<f32>&        _upstream;  ///< 上游管道引用
    TrapezoidTrajectoryConfig _config;    ///< 配置参数
    Storage&                  _storage;   ///< 外部存储

    /**
     * @brief 更新轨迹输出值
     * @param setPoint 设定值
     */
    void updateTrajectory(f32 setPoint) {
        // 检查设定值是否发生变化
        if (std::abs(setPoint - _storage.setPoint) > 1e-9f) {
            // 设定值改变，重新规划轨迹
            _storage.setPoint = setPoint;
            calculateTrajectory(_storage.output, setPoint);
        }

        // 根据当前阶段更新输出
        switch (_storage.phase) {
            case TrapezoidPhase::kIdle:
            case TrapezoidPhase::kCompleted:
                // 空闲或完成状态，输出不变
                break;

            case TrapezoidPhase::kAcceleration: {
                // 加速段
                _storage.phaseTimer += _config.sampleTime;

                if (_storage.phaseTimer >= _storage.accelTime) {
                    // 加速段结束，进入匀速段
                    _storage.phase      = TrapezoidPhase::kConstant;
                    _storage.phaseTimer = 0.0f;
                    _storage.velocity   = _config.maxVelocity * _storage.direction;
                    _storage.output =
                        _storage.startPosition + _storage.accelDistance * _storage.direction;
                } else {
                    // 继续加速
                    f32 t             = _storage.phaseTimer;
                    _storage.velocity = _config.acceleration * t * _storage.direction;
                    _storage.output   = _storage.startPosition +
                                      0.5f * _config.acceleration * t * t * _storage.direction;
                }
                break;
            }

            case TrapezoidPhase::kConstant: {
                // 匀速段
                _storage.phaseTimer += _config.sampleTime;

                if (_storage.phaseTimer >= _storage.constantTime) {
                    // 匀速段结束，进入减速段
                    _storage.phase      = TrapezoidPhase::kDeceleration;
                    _storage.phaseTimer = 0.0f;
                    _storage.output =
                        _storage.startPosition +
                        (_storage.accelDistance + _storage.constantDistance) * _storage.direction;
                } else {
                    // 继续匀速
                    f32 t = _storage.phaseTimer;
                    _storage.output =
                        _storage.startPosition +
                        (_storage.accelDistance + _config.maxVelocity * t) * _storage.direction;
                }
                break;
            }

            case TrapezoidPhase::kDeceleration: {
                // 减速段
                _storage.phaseTimer += _config.sampleTime;

                if (_storage.phaseTimer >= _storage.decelTime) {
                    // 减速段结束，到达目标
                    _storage.phase    = TrapezoidPhase::kCompleted;
                    _storage.velocity = 0.0f;
                    _storage.output   = _storage.targetPosition;
                } else {
                    // 继续减速
                    f32 t = _storage.phaseTimer;
                    f32 decelDistance =
                        _config.maxVelocity * t - 0.5f * _config.deceleration * t * t;
                    _storage.velocity =
                        (_config.maxVelocity - _config.deceleration * t) * _storage.direction;
                    _storage.output =
                        _storage.startPosition +
                        (_storage.accelDistance + _storage.constantDistance + decelDistance) *
                            _storage.direction;
                }
                break;
            }
        }

        // 限制输出值在合理范围内
        _storage.output = clampValue(_storage.output);
    }

    /**
     * @brief 计算梯形轨迹参数
     * @param start 起始位置
     * @param target 目标位置
     */
    void calculateTrajectory(f32 start, f32 target) {
        _storage.startPosition  = start;
        _storage.targetPosition = target;

        // 计算总距离和方向
        f32 totalDistance  = target - start;
        _storage.direction = (totalDistance >= 0) ? 1.0f : -1.0f;
        totalDistance      = std::abs(totalDistance);

        // 如果距离很小，直接完成
        if (totalDistance < 1e-6f) {
            _storage.phase    = TrapezoidPhase::kCompleted;
            _storage.velocity = 0.0f;
            _storage.output   = target;
            return;
        }

        // 计算加速和减速所需的距离
        f32 accelDistance =
            (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.acceleration);
        f32 decelDistance =
            (_config.maxVelocity * _config.maxVelocity) / (2.0f * _config.deceleration);

        // 检查是否能达到最大速度（三角形轨迹 vs 梯形轨迹）
        if (accelDistance + decelDistance > totalDistance) {
            // 三角形轨迹：无法达到最大速度
            f32 peakVelocity =
                std::sqrt(2.0f * totalDistance * _config.acceleration * _config.deceleration /
                          (_config.acceleration + _config.deceleration));

            _storage.accelDistance = (peakVelocity * peakVelocity) / (2.0f * _config.acceleration);
            _storage.decelDistance = (peakVelocity * peakVelocity) / (2.0f * _config.deceleration);
            _storage.constantDistance = 0.0f;

            _storage.accelTime    = peakVelocity / _config.acceleration;
            _storage.constantTime = 0.0f;
            _storage.decelTime    = peakVelocity / _config.deceleration;
        } else {
            // 梯形轨迹：能达到最大速度
            _storage.accelDistance    = accelDistance;
            _storage.decelDistance    = decelDistance;
            _storage.constantDistance = totalDistance - accelDistance - decelDistance;

            _storage.accelTime    = _config.maxVelocity / _config.acceleration;
            _storage.constantTime = _storage.constantDistance / _config.maxVelocity;
            _storage.decelTime    = _config.maxVelocity / _config.deceleration;
        }

        // 开始轨迹执行
        _storage.phase      = TrapezoidPhase::kAcceleration;
        _storage.phaseTimer = 0.0f;
        _storage.velocity   = 0.0f;
    }

    /**
     * @brief 限制数值在合理范围内
     * @param value 输入值
     * @return 限制后的值
     */
    f32 clampValue(f32 value) const {
        // 检查是否为有效数值
        if (std::isnan(value) || std::isinf(value)) {
            return 0.0f;
        }

        // 限制在合理范围内，防止数值溢出
        const f32 maxValue = 1e6f;
        const f32 minValue = -1e6f;

        return std::max(minValue, std::min(maxValue, value));
    }
};

}  // namespace wibot