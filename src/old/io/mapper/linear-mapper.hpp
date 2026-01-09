#pragma once

#include "model.hpp"

namespace wibot {

/**
 * @brief 线性映射管道
 * 
 * 将int16_t输入范围线性映射到float输出范围。
 * 公式：output = (input - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin
 * 
 * 配置使用引用方式，支持多个映射器实例共享同一配置。
 */
class LinearMapper : public SyncPipeline<f32> {
   public:
    /**
     * @brief 线性映射配置
     */
    struct Config {
        f32  inputMin;     ///< 输入最小值
        f32  inputMax;     ///< 输入最大值
        f32  outputMin;    ///< 输出最小值
        f32  outputMax;    ///< 输出最大值
        bool clampOutput;  ///< 是否限制输出范围
    };

   public:
    /**
     * @brief 构造线性映射器
     * 
     * @param upstream 上游管道
     * @param config 映射配置（引用方式，支持共享）
     */
    LinearMapper(SyncPipeline<i16>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
    }

    f32 getValue() const override {
        // 获取上游值并实时处理
        i16 input = _upstream.getValue();
        return _mapLinear(static_cast<f32>(input));
    }

    void reset() override {
        _upstream.reset();
    }

    void update() override {
        _upstream.update();
    }

    /**
     * @brief 更新映射配置
     */
   private:
    /**
     * @brief 执行线性映射
     */
    f32 _mapLinear(f32 input) const {
        if (_config.inputMax == _config.inputMin) {
            return _config.outputMin;
        }

        f32 ratio  = (input - _config.inputMin) / (_config.inputMax - _config.inputMin);
        f32 result = _config.outputMin + ratio * (_config.outputMax - _config.outputMin);

        // 限制输出范围（如果启用）
        if (_config.clampOutput) {
            if (result < _config.outputMin) result = _config.outputMin;
            if (result > _config.outputMax) result = _config.outputMax;
        }

        return result;
    }

   private:
    SyncPipeline<i16>& _upstream;  ///< 上游管道引用
    const Config&      _config;    ///< 映射配置引用（支持共享）
};

}  // namespace wibot
