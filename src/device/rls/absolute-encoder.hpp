#pragma once

#include "dsp/filter/iir.hpp"

namespace wibot {

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
        f32 samplePeriod;  // 最小采样周期 (s)

        // 滤波器参数
        f32 highTrackingCycles;  // 高速段跟踪周期数 (采样周期数)，默认5
        f32 lowTrackingCycles;   // 低速段跟踪周期数 (有效采样周期数)，默认2
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
     * @param position 初始位置 (ticks)
     * @param speed 初始速度 (ticks/s)
     */
    void reset(i32 position, f32 speed);

    /**
     * @brief 应用配置参数
     */
    void applyConfig();

    /**
     * @brief 获取当前位置 (ticks)
     */
    i32 getPosition() const {
        return _position;
    }

    /**
     * @brief 获取当前速度 (ticks/s)
     */
    f32 getSpeed() const {
        return _speed;
    }

    /**
     * @brief 获取换算后的角度值 (弧度)
     */
    f32 getAngular() const;

    /**
     * @brief 获取换算后的角速度 (弧度/s)
     */
    f32 getAngularSpeed() const;

    explicit AbsoluteEncoder(Config& config) : _config(config) {
        applyConfig();
    }

   private:
    Config&     _config;
    IIR::Config _highLpConfig{};
    IIR::Config _lowLpConfig{};

    // 低通滤波器（高低速段）
    IIR _highLp{_highLpConfig};
    IIR _lowLp{_lowLpConfig};

    // 位置和速度跟踪
    u32 _lastValue = 0;

    i32 _position = 0;     // 当前位置 (ticks)
    f32 _speed    = 0.0f;  // 当前速度 (ticks/s)

    // 计算参数（从Config计算得出）
    f32 _dispThreshold = 0.0f;  // 位移切换阈值 (ticks)

    // 位移/时间积累计数器（用于低速段）
    i32 _lowSpeedAccumulator = 0;     // 低速段位移累计器 (ticks)
    f32 _lowSpeedTimeAccum   = 0.0f;  // 低速段时间累计器 (s)

   private:
    /**
     * @brief 处理位移，考虑环绕
     */
    i32 _calculateDisplacement(u32 currentValue);

    /**
     * @brief 获取混合因子 (0=全低速, 1=全高速)
     */
    f32 _getMixingFactor(f32 displacement);

    /**
     * @brief 计算高速段速度（差分法）
     */
    f32 _calculateHighSpeedVelocity(i32 displacement, f32 samplePeriod);

    /**
     * @brief 计算低速段速度（累计法）
     */
    f32 _calculateLowSpeedVelocity(i32 accumulatedDispl);

    static IIR::Config _getInitialLpConfig() {
        IIR::Config cfg{};
        cfg.samplePeriod = 0.001f;  // 默认值，会在 applyConfig 中覆盖
        cfg.cutoffFreq   = 1.0f;
        cfg.wrapValue    = 0.0f;
        return cfg;
    }
};
}  // namespace wibot
