#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 一阶IIR低通滤波器
 * 
 * 面向对象实现，支持非周期和周期数据。
 * 支持两种工作模式：
 * - 固定采样周期模式：samplePeriod在Config中设定，系数预计算，每次调用filter(input)
 * - 可变采样周期模式：samplePeriod由调用者传入，每次filter调用时动态计算系数
 */
class IIR {
   public:
    struct Config {
        f32 samplePeriod;  // 采样周期（秒）。固定模式下用于预计算；可变模式下作为备用默认值
        f32 cutoffFreq;    // 截止频率（Hz）
        f32 wrapValue;     // 周期（P）。当>0时启用周期数据处理；输入范围要求为 [0, P)
    };

    /**
     * @brief 构造函数
     * @param cfg 滤波配置（支持constexpr或运行时可修改配置）
     * @note 支持两种场景：
     *       1. constexpr Config 用于运行时无需修改的参数
     *       2. 非const Config 用于运行时需要修改的参数
     */
    explicit IIR(const Config& cfg);

    /**
     * @brief 应用配置（当外部修改了引用指向的配置对象时调用）
     * @return 是否成功（配置验证失败时返回false，原系数保持不变）
     */
    bool applyConfig();

    /**
     * @brief 处理单个样本（固定采样周期模式）
     * @param input 输入样本
     * @return 滤波后的输出
     * @note 使用Config中预计算的采样周期系数
     *       模式：固定采样周期。Config中的samplePeriod在构造时已预计算系数
     */
    f32 filter(f32 input);

    /**
     * @brief 处理单个样本（可变采样周期模式）
     * @param input 输入样本
     * @param samplePeriod 当前采样间隔（秒）
     * @return 滤波后的输出
     * @note 模式：可变采样周期。每次调用根据samplePeriod重新计算系数
     *       适用于采样周期动态变化的场景（如jitter采样、事件驱动）
     */
    f32 filter(f32 input, f32 samplePeriod);

    /**
     * @brief 重置状态
     */
    void reset();

    /**
     * @brief 验证配置有效性
     */
    static bool isConfigValid(const Config& cfg);

   private:
    void _updateCoefficients();

    f32 _computeAlpha(f32 dt) const;

    // 将值归一化到 [0, period) 区间
    static f32 _wrapPeriod(f32 x, f32 period);
    // 将差值归一化到 (-period/2, period/2] 区间（最短路径差）
    static f32 _wrapDiff(f32 d, f32 period);

   private:
    const Config& _config;
    f32           _alpha;
    f32           _one_minus_alpha;
    f32           _y_last;
    bool          _first;
};

}  // namespace wibot
