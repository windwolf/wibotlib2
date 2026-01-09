#pragma once

#include "type.hpp"

namespace wibot::dsp {

/**
 * @brief 一阶低通滤波器
 * 
 * 面向对象实现，支持非周期和周期数据。
 */
class Lowpass {
   public:
    struct Config {
        f32 sampleTime;  // 采样间隔（秒）
        f32 cutoffFreq;  // 截止频率（Hz）
        f32 wrapValue;   // 折叠值（周期性数据），0表示禁用
    };

    /**
     * @brief 构造函数
     * @param cfg 滤波配置
     */
    explicit Lowpass(const Config& cfg);

    /**
     * @brief 处理单个样本
     * @return 滤波后的输出
     */
    f32 filter(f32 input);

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

    static f32 _wrap(f32 x, f32 w);

   private:
    const Config& _config;
    f32           _alpha;
    f32           _one_minus_alpha;
    f32           _y_last;
    bool          _first;
};

}  // namespace wibot::dsp
