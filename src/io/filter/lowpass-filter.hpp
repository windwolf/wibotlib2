#pragma once

#include "../model.hpp"
#include <cmath>

namespace wibot {

/**
 * @brief 一阶低通滤波器管道
 * 
 * 将float输入值进行一阶低通滤波处理。
 * 
 * 采用一阶RC滤波器模型：
 * - 非周期性数据：y[n] = α * x[n] + (1-α) * y[n-1]
 * - 周期性数据：y[n] = wrap(y[n-1] + α * wrap(x[n] - y[n-1]))
 * 其中 α = 2πfc*T / (1 + 2πfc*T)，fc为截止频率，T为采样间隔
 * 
 * 配置使用引用方式，支持多个滤波器实例共享同一配置。
 */
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
     * @brief 构造低通滤波器
     * 
     * @param upstream 上游管道
     * @param config 滤波配置（引用方式，支持共享）
     */
    LowpassFilter(SyncPipeline<f32>& upstream, const Config& config)
        : _upstream(upstream), _config(config), _firstUpdate(true) {
        _calculateFilterCoefficients();
        _outputLast = 0.0f;
    }

    f32 getValue() const override {
        return _outputLast;
    }

    void reset() override {
        _outputLast  = 0.0f;
        _firstUpdate = true;
        _upstream.reset();
    }

    void update() override {
        _upstream.update();
        f32 input = _upstream.getValue();

        if (_firstUpdate) {
            // 第一次更新：直接使用输入值作为初始输出
            _outputLast  = input;
            _firstUpdate = false;
        } else {
            // 后续更新：进行滤波处理
            _outputLast = _filterValue(input);
        }
    }

    /**
     * @brief 验证配置是否有效
     */
    static bool isConfigValid(const Config& config) {
        if (config.sampleTime <= 0.0f || config.cutoffFreq <= 0.0f) {
            return false;
        }

        // 检查奈奎斯特定理：截止频率不能超过采样频率的一半
        f32 sampleFreq = 1.0f / config.sampleTime;
        if (config.cutoffFreq >= sampleFreq / 2.0f) {
            return false;
        }

        if (config.wrapValue < 0.0f) {
            return false;
        }

        return true;
    }

   private:
    /**
     * @brief 计算滤波系数
     */
    void _calculateFilterCoefficients() {
        const f32 PI     = 3.14159265359f;
        f32       omegaT = 2.0f * PI * _config.cutoffFreq * _config.sampleTime;

        _alpha   = omegaT / (1.0f + omegaT);
        _1_alpha = 1.0f - _alpha;
    }

    /**
     * @brief 对输入值进行滤波处理
     */
    f32 _filterValue(f32 input) {
        if (_config.wrapValue <= 0.0f) {
            // 标准一阶低通滤波
            return _alpha * input + _1_alpha * _outputLast;
        } else {
            // 使用wrap逻辑处理周期性数据
            f32 diff              = input - _outputLast;
            f32 wrappedDiff       = _wrap(diff, _config.wrapValue);
            f32 filteredIncrement = _alpha * wrappedDiff;
            return _wrap(_outputLast + filteredIncrement, _config.wrapValue);
        }
    }

    /**
     * @brief 折叠函数（用于周期性数据）
     */
    f32 _wrap(f32 x, f32 w) const {
        return x - 2.0f * w * std::floor((x + w) / (2.0f * w));
    }

   private:
    SyncPipeline<f32>& _upstream;  ///< 上游管道引用
    const Config&      _config;    ///< 滤波配置引用（支持共享）

    // 滤波系数
    f32 _alpha;    ///< 滤波系数 α
    f32 _1_alpha;  ///< 滤波系数 1-α

    // 滤波状态
    f32  _outputLast;   ///< 上次的输出值
    bool _firstUpdate;  ///< 是否为首次更新
};

}  // namespace wibot
