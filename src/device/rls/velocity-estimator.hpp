#pragma once

#include "base/type.hpp"
#include "dsp/filter/iir.hpp"

namespace wibot {

/**
 * @brief 速度估计器
 * 
 * 根据位置信息计算速度，使用高速段（差分法）和低速段（累计法）混合策略，
 * 并进行滤波处理
 */
class VelocityEstimator {
   public:
    struct Config {
        // 编码器物理参数（用于角度转换）
        u32 resolution;  // 编码器分辨率 (ticks/rev)

        // 采样参数
        f32 samplePeriod;  // 采样周期 (s)

        // 滤波器参数
        f32 trackingCycles;  // 追踪周期数（IIR滤波器时间常数），默认5
    };

   public:
    /**
     * @brief 使用固定采样周期更新速度估计
     * 
     * @param displacement 位移 (ticks)，由外部累计后传入
     */
    void update(i32 displacement);

    /**
     * @brief 使用可变采样周期更新速度估计
     *
     * @param displacement 位移 (ticks)，由外部累计后传入
     * @param samplePeriod 采样周期 (s)
     */
    void update(i32 displacement, f32 samplePeriod);

    /**
     * @brief 重置速度估计器
     * 
     * @param speed 初始速度 (ticks/s)
     */
    void reset(f32 speed = 0.0f);

    /**
     * @brief 应用配置参数
     */
    void applyConfig();

    /**
     * @brief 获取当前速度 (ticks/s)
     */
    f32 getSpeed() const {
        return _speed;
    }

    /**
     * @brief 获取换算后的角速度 (弧度/s)
     */
    f32 getAngularSpeed() const;

    explicit VelocityEstimator(Config& config) : _config(config) {
        applyConfig();
    }

   private:
    Config&     _config;
    IIR::Config _lpConfig{};

    // 低通滤波器
    IIR _lpFilter{_lpConfig};

    // 速度跟踪
    f32 _speed = 0.0f;  // 当前速度 (ticks/s)

   private:
    static IIR::Config _getInitialLpConfig() {
        IIR::Config cfg{};
        cfg.samplePeriod = 0.001f;  // 默认值，会在 applyConfig 中覆盖
        cfg.cutoffFreq   = 1.0f;
        cfg.wrapValue    = 0.0f;
        return cfg;
    }
};

}  // namespace wibot
