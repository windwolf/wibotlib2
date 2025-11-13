#pragma once

#include "../model.hpp"
#include <cmath>

namespace wibot {

/**
 * @brief 一阶低通滤波器管道
 * 
 * 将float输入值进行一阶低通滤波处理。支持多通道实时状态滤波。
 * 所有通道共享同一套滤波配置，但各通道维护独立的滤波状态。
 * 
 * 采用一阶RC滤波器模型：
 * - 非周期性数据：y[n] = α * x[n] + (1-α) * y[n-1]
 * - 周期性数据：y[n] = wrap(y[n-1] + α * wrap(x[n] - y[n-1]))
 * 其中 α = 2πfc*T / (1 + 2πfc*T)，fc为截止频率，T为采样间隔
 * 
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <u8 CHANNELS>
class LowpassFilter : public SyncPipeline<f32> {
   public:
    /**
     * @brief 一阶低通滤波器配置
     */
    struct Config {
        f32 sampleTime;  ///< 采样间隔（秒）
        f32 cutoffFreq;  ///< 截止频率（Hz）
        f32 wrapValue;   ///< 折叠值（用于角度等周期性数据，0表示不启用）
    };

   public:
    /**
     * @brief 构造低通滤波器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的滤波配置
     */
    LowpassFilter(SyncPipeline<f32>& upstream, const Config& config);

    /**
     * @brief 获取滤波后的值
     */
    f32 getValue(u8 channel) const override;

    /**
     * @brief 重置管道状态
     */
    void reset() override;

    /**
     * @brief 更新管道状态
     */
    void update() override;

    /**
     * @brief 更新滤波配置（影响所有通道）
     * 
     * @param config 新的滤波配置
     * @note 更新配置会重新计算滤波系数
     */
    void updateConfig(const Config& config);

    /**
     * @brief 验证配置是否有效
     * 
     * @param config 要验证的配置
     * @return true 配置有效
     * @return false 配置无效（采样时间或截止频率非正值等）
     */
    static bool isConfigValid(const Config& config);

   private:
    /**
     * @brief 计算滤波系数
     */
    void _calculateFilterCoefficients();

    /**
     * @brief 对单个输入值进行滤波处理
     * 
     * @param input 输入值
     * @param channel 通道索引
     * @return f32 滤波后的输出值
     */
    f32 _filterValue(f32 input, u8 channel);

    /**
     * @brief 折叠函数（用于周期性数据）
     * 
     * @param x 待折叠的值
     * @param w 折叠范围的半径
     * @return f32 折叠后的值，范围在 [-w, w] 内
     */
    f32 _wrap(f32 x, f32 w) const;

   private:
    SyncPipeline<f32>& _upstream;  ///< 上游管道引用
    Config             _config;    ///< 共享的滤波配置

    // 滤波系数
    f32 _alpha;    ///< 滤波系数 α
    f32 _1_alpha;  ///< 滤波系数 1-α

    // 各通道的滤波状态
    f32  _outputLast[CHANNELS];  ///< 各通道上次的输出值
    bool _firstUpdate;           ///< 是否为首次更新（所有通道共享）
};

// ============================================================================
// LowpassFilter 模板实现
// ============================================================================

template <u8 CHANNELS>
LowpassFilter<CHANNELS>::LowpassFilter(SyncPipeline<f32>& upstream, const Config& config)
    : _upstream(upstream), _config(config) {
    // 验证配置有效性
    if (!isConfigValid(config)) {
        // 如果配置无效，使用默认配置
        _config.sampleTime = 0.01f;  // 默认10ms采样间隔
        _config.cutoffFreq = 10.0f;  // 默认10Hz截止频率
        _config.wrapValue  = 0.0f;   // 默认不启用折叠
    }

    // 计算滤波系数
    _calculateFilterCoefficients();

    // 初始化各通道的状态
    for (u8 i = 0; i < CHANNELS; i++) {
        _outputLast[i] = 0.0f;  // 初始输出值
    }
    _firstUpdate = true;  // 标记为首次更新
}

template <u8 CHANNELS>
f32 LowpassFilter<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 无效通道返回0
    }

    return _outputLast[channel];
}

template <u8 CHANNELS>
void LowpassFilter<CHANNELS>::reset() {
    // 重置各通道的滤波状态
    for (u8 i = 0; i < CHANNELS; i++) {
        _outputLast[i] = 0.0f;  // 重置输出值
    }
    _firstUpdate = true;  // 标记为首次更新

    // 重置上游管道
    _upstream.reset();
}

template <u8 CHANNELS>
void LowpassFilter<CHANNELS>::update() {
    // 更新上游管道
    _upstream.update();

    // 对所有通道进行滤波处理
    for (u8 i = 0; i < CHANNELS; i++) {
        f32 input = _upstream.getValue(i);

        if (_firstUpdate) {
            // 第一次更新：直接使用输入值作为初始输出
            _outputLast[i] = input;
        } else {
            // 后续更新：进行滤波处理
            _outputLast[i] = _filterValue(input, i);
        }
    }

    // 第一次更新完成后，标记为非首次更新
    if (_firstUpdate) {
        _firstUpdate = false;
    }
}

template <u8 CHANNELS>
void LowpassFilter<CHANNELS>::updateConfig(const Config& config) {
    if (isConfigValid(config)) {
        _config = config;
        _calculateFilterCoefficients();

        // 如果初始值发生变化，可选择重置状态
        // 这里保持当前状态，只更新滤波参数
    }
    // 如果配置无效，保持原有配置不变
}

template <u8 CHANNELS>
bool LowpassFilter<CHANNELS>::isConfigValid(const Config& config) {
    // 检查采样时间是否为正值
    if (config.sampleTime <= 0.0f) {
        return false;
    }

    // 检查截止频率是否为正值
    if (config.cutoffFreq <= 0.0f) {
        return false;
    }

    // 检查奈奎斯特定理：截止频率不能超过采样频率的一半
    f32 sampleFreq = 1.0f / config.sampleTime;
    if (config.cutoffFreq >= sampleFreq / 2.0f) {
        return false;
    }

    // 检查折叠值（如果启用的话）
    if (config.wrapValue < 0.0f) {
        return false;
    }

    return true;
}

template <u8 CHANNELS>
void LowpassFilter<CHANNELS>::_calculateFilterCoefficients() {
    // 计算一阶低通滤波器系数
    // α = 2πfc*T / (1 + 2πfc*T)
    // 其中 fc 是截止频率，T 是采样间隔

    const f32 PI     = 3.14159265359f;
    f32       omegaT = 2.0f * PI * _config.cutoffFreq * _config.sampleTime;

    _alpha   = omegaT / (1.0f + omegaT);
    _1_alpha = 1.0f - _alpha;
}

template <u8 CHANNELS>
f32 LowpassFilter<CHANNELS>::_filterValue(f32 input, u8 channel) {
    if (_config.wrapValue <= 0.0f) {
        // 无折叠值：标准一阶低通滤波
        return _alpha * input + _1_alpha * _outputLast[channel];
    } else {
        // 有折叠值：使用wrap逻辑处理周期性数据
        // 1. 计算输入与上次输出的差值
        f32 diff = input - _outputLast[channel];

        // 2. 对差值进行wrap处理
        f32 wrappedDiff = _wrap(diff, _config.wrapValue);

        // 3. 对wrap后的差值进行滤波
        f32 filteredIncrement = _alpha * wrappedDiff;

        // 4. 将滤波后的增量加到上次输出上，然后对结果进行wrap
        return _wrap(_outputLast[channel] + filteredIncrement, _config.wrapValue);
    }
}

template <u8 CHANNELS>
f32 LowpassFilter<CHANNELS>::_wrap(f32 x, f32 w) const {
    // 实现与原版相同的wrap函数
    // 将值x折叠到 [-w, w] 范围内
    return x - 2.0f * w * std::floor((x + w) / (2.0f * w));
}

}  // namespace wibot
