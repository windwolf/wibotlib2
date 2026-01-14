#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 移动平均（均值）低通滤波器
 *
 * 支持：
 * - 固定采样周期：构造或 applyConfig 时按截止频率计算窗口大小
 * - 可变采样周期：惰性更新（periodTolerance），仅在周期变化超过阈值时重算窗口
 * - 周期数据：相对当前样本进行折叠差分平均，避免跨界跳变
 */
class MovingAverage {
   public:
    struct Config {
        f32 samplePeriod;     // 采样周期（秒），用于初始窗口计算
        f32 cutoffFreq;       // 截止频率（Hz），用于估算窗口大小
        f32 wrapValue;        // 折叠值（周期性数据），0 表示禁用
        f32 periodTolerance;  // 可变周期：相对变化容忍度（如 0.05 表示 5%）
    };

    explicit MovingAverage(const Config& cfg);

    bool applyConfig();

    f32 filter(f32 input);

    f32 filter(f32 input, f32 samplePeriod);

    void reset();

    static bool isConfigValid(const Config& cfg);

   private:
    static constexpr u16 MAX_TAPS = 64;

    u16 _computeTapCount(f32 samplePeriod) const;  // 根据 fc 与 fs 估算窗口

    void _designWindow(f32 samplePeriod);  // 计算窗口大小与系数

    static f32 _wrap(f32 x, f32 w);

   private:
    const Config& _config;
    f32           _xbuf[MAX_TAPS]{};
    u16           _tapCount{1};
    u16           _idx{0};
    f32           _sum{0.0f};     // 非周期数据的运行和
    f32           _invTap{1.0f};  // 1/_tapCount
    f32           _lastDesignPeriod{0.0f};
    bool          _first{true};
};

}  // namespace wibot
