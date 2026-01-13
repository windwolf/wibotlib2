#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 线性相位低通 FIR 滤波器
 *
 * 支持：
 * - 固定采样周期：构造或 applyConfig 时预计算系数
 * - 可变采样周期：惰性更新策略，仅在周期变化超过容忍度时才重算（默认5%）
 * - 周期数据：通过相对参考值进行折叠差分，避免跨界跳变
 */
class FIR {
   public:
    struct Config {
        f32 samplePeriod;  // 采样周期（秒）。固定模式预计算；可变模式备用默认值
        f32 cutoffFreq;    // 截止频率（Hz）
        f32 wrapValue;     // 折叠值（周期性数据），0 表示禁用
        u16 tapCount;      // 滤波器抽头数量（建议奇数）
        f32 periodTolerance;  // 可变周期模式：变化容忍度（相对值，如0.05表示5%），低于此值不重算系数。0表示禁用惰性更新
    };

    explicit FIR(const Config& cfg);

    bool applyConfig();

    f32 filter(f32 input);

    f32 filter(f32 input, f32 samplePeriod);

    void reset();

    static bool isConfigValid(const Config& cfg);

   private:
    static constexpr u16 MAX_TAPS = 64;

    void _designCoeffs(f32 samplePeriod);

    static f32 _sinc(f32 x);

    static f32 _wrap(f32 x, f32 w);

   private:
    const Config& _config;
    f32           _coeffs[MAX_TAPS]{};
    f32           _xbuf[MAX_TAPS]{};
    u16           _tapCount{0};
    u16           _idx{0};
    f32           _lastDesignPeriod{0.0f};  // 上次系数设计时的采样周期
    bool          _first{true};
};

}  // namespace wibot
