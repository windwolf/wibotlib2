#pragma once

#include "dsp/filter/iir.hpp"

namespace wibot {

class AbsoluteEncoder {
   public:
    struct Config {
        // 编码器物理参数
        u32 resolution;  // 编码器分辨率 (ticks/rev)

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
     * @param wrapped 编码器值是否按 resolution 折叠（默认false）
     */
    void update(u32 value, bool wrapped = false);

    /**
     * @brief 使用可变采样周期更新编码器位置和速度
     *
     * @param value 当前编码器读数 (ticks)
     * @param samplePeriod 当前采样周期 (s)
     * @param wrapped 指示编码器值`value`是否按 resolution 折叠（默认false）
     */
    void update(u32 value, f32 samplePeriod, bool wrapped = false);

    /**
     * @brief 重置编码器状态
     * 
     * @param position 初始位置 (ticks)
     * @param speed 初始速度 (ticks/s)
     */
    void reset(f32 position, f32 speed);

    /**
     * @brief 应用配置参数
     */
    void applyConfig();

    /**
     * @brief 获取当前位置 (ticks)
     */
    f32 getPosition() const {
        return position_;
    }

    /**
     * @brief 获取当前速度 (ticks/s)
     */
    f32 getSpeed() const {
        return speed_;
    }

    /**
     * @brief 获取换算后的角度值 (弧度)
     */
    f32 getAngular() const;

    /**
     * @brief 获取换算后的角速度 (弧度/s)
     */
    f32 getAngularSpeed() const;

    explicit AbsoluteEncoder(Config& config)
        : _config(config), _highLp(_getInitialLpConfig()), _lowLp(_getInitialLpConfig()) {
        applyConfig();
    }

   private:
    Config&     _config;
    IIR::Config _highLpConfig{};
    IIR::Config _lowLpConfig{};

    // 位置和速度跟踪
    u32 lastValue_               = 0;
    i64 accumulatedDisplacement_ = 0;  // 累计位移 (ticks)

    f32 position_ = 0.0f;  // 当前位置 (ticks)
    f32 speed_    = 0.0f;  // 当前速度 (ticks/s)

    // 低通滤波器（高低速段）
    IIR _highLp{_highLpConfig};
    IIR _lowLp{_lowLpConfig};

    // 计算参数（从Config计算得出）
    f32 dispThreshold_ = 0.0f;  // 位移切换阈值 (ticks)

    // 位移/时间积累计数器（用于低速段）
    i64 lowSpeedAccumulator_ = 0;     // 低速段位移累计器 (ticks)
    f32 lowSpeedTimeAccum_   = 0.0f;  // 低速段时间累计器 (s)

   private:
    /**
     * @brief 处理位移，考虑环绕
     */
    i32 calculateDisplacement_(u32 currentValue);

    /**
     * @brief 获取混合因子 (0=全低速, 1=全高速)
     */
    f32 getMixingFactor_(f32 displacement);

    /**
     * @brief 计算高速段速度（差分法）
     */
    f32 calculateHighSpeedVelocity_(i32 displacement, f32 samplePeriod);

    /**
     * @brief 计算低速段速度（累计法）
     */
    f32 calculateLowSpeedVelocity_(i64 accumulatedDispl);

    static IIR::Config _getInitialLpConfig() {
        IIR::Config cfg{};
        cfg.samplePeriod = 0.001f;  // 默认值，会在 applyConfig 中覆盖
        cfg.cutoffFreq   = 1.0f;
        cfg.wrapValue    = 0.0f;
        return cfg;
    }
};
}  // namespace wibot
