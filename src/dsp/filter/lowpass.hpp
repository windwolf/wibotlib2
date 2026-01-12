#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 一阶低通滤波器
 * 
 * 面向对象实现，支持非周期和周期数据。
 */
class Lowpass {
   public:
    struct Config {
        f32 samplePeriod;  // 默认采样周期（秒），用于等间隔采样
        f32 cutoffFreq;    // 截止频率（Hz）
        f32 wrapValue;     // 折叠值（周期性数据），0表示禁用
    };

    /**
     * @brief 构造函数
     * @param cfg 滤波配置（支持constexpr或运行时可修改配置）
     * @note 支持两种场景：
     *       1. constexpr Config 用于运行时无需修改的参数
     *       2. 非const Config 用于运行时需要修改的参数
     */
    explicit Lowpass(const Config& cfg);

    /**
     * @brief 应用配置（当外部修改了引用指向的配置对象时调用）
     * @return 是否成功（配置验证失败时返回false，原系数保持不变）
     */
    bool applyConfig();

    /**
     * @brief 处理单个样本（等间隔采样）
     * @param input 输入样本
     * @return 滤波后的输出
     * @note 使用Config中的samplePeriod作为采样周期
     */
    f32 filter(f32 input);

    /**
     * @brief 处理单个样本（不等间隔采样）
     * @param input 输入样本
     * @param samplePeriod 当前采样间隔（秒）
     * @return 滤波后的输出
     * @note 适用于采样周期变化的场景，每次根据dt重新计算滤波系数
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

    static f32 _wrap(f32 x, f32 w);

   private:
    const Config& _config;
    f32           _alpha;
    f32           _one_minus_alpha;
    f32           _y_last;
    bool          _first;
};

}  // namespace wibot
