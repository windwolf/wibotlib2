#pragma once

#include "position-tracker.hpp"
#include "velocity-estimator.hpp"

namespace wibot {

/**
 * @brief 绝对式编码器组件
 * 
 * 提供位置和速度信息的完整解决方案，内部组合使用：
 * - PositionTracker: 处理编码器值到位置的转换（展开折叠）
 * - VelocityEstimator: 处理位置到速度的估计和滤波
 */
class AbsoluteEncoder {
   public:
    struct Config {
        // 编码器物理参数
        u32 resolution;  // 编码器分辨率 (ticks/rev)

        // 输入数据折叠特性
        u32 inputWrapRange;  // 输入折叠范围；为0表示无需折叠

        // 速度范围 (单位: ticks/s)
        f32 maxSpeed;  // 最大速度
        f32 minSpeed;  // 最小速度

        // 采样参数
        f32 samplePeriod;  // 采样周期 (s)

        // 滤波器参数
        f32 trackingCycles;  // 追踪周期数 (IIR滤波器时间常数)，默认5

        // 位移累计阈值
        i32 minDisplacementThreshold;  // 最小位移阈值 (ticks)，小于此值时暂不更新速度，default=1
    };

   public:
    /**
     * @brief 使用固定采样周期更新编码器位置和速度
     * 
     * @param value 当前编码器读数 (ticks)
     */
    void update(u32 value);

    /**
     * @brief 使用可变采样周期更新编码器位置和速度
     *
     * @param value 当前编码器读数 (ticks)
     * @param samplePeriod 当前采样周期 (s)
     */
    void update(u32 value, f32 samplePeriod);

    /**
     * @brief 重置编码器状态
     * 
     * @param value 当前编码器读数 (ticks)
     * @param position 初始位置 (ticks)
     * @param speed 初始速度 (ticks/s)
     */
    void reset(u32 value, i32 position = 0, f32 speed = 0.0f);

    /**
     * @brief 获取当前位置 (ticks)
     */
    i32 getPosition() const {
        return _posTracker.getPosition();
    }

    /**
     * @brief 获取当前速度 (ticks/s)
     */
    f32 getSpeed() const {
        return _velEstimator.getSpeed();
    }

    /**
     * @brief 获取换算后的角度值 (弧度)
     */
    f32 getAngular() const {
        return _posTracker.getAngular();
    }

    /**
     * @brief 获取换算后的角速度 (弧度/s)
     */
    f32 getAngularSpeed() const {
        return _velEstimator.getAngularSpeed();
    }

    /**
     * @brief 获取位置跟踪器引用
     */
    PositionTracker& getPositionTracker() {
        return _posTracker;
    }

    /**
     * @brief 获取速度估计器引用
     */
    VelocityEstimator& getVelocityEstimator() {
        return _velEstimator;
    }

    explicit AbsoluteEncoder(Config& config)
        : _config(config),
          _posTrackerConfig{config.resolution, config.inputWrapRange},
          _velEstimatorConfig{config.resolution, config.samplePeriod, config.trackingCycles},
          _posTracker(_posTrackerConfig),
          _velEstimator(_velEstimatorConfig) {
    }

   private:
    Config& _config;

    // 子组件配置
    PositionTracker::Config   _posTrackerConfig;
    VelocityEstimator::Config _velEstimatorConfig;

    // 子组件
    PositionTracker   _posTracker;
    VelocityEstimator _velEstimator;

    // 位移累计（低于阈值时）
    i32 _accumulatedDisplacement = 0;     // 累计位移 (ticks)
    f32 _accumulatedTime         = 0.0f;  // 累计时间 (s)
};
}  // namespace wibot
