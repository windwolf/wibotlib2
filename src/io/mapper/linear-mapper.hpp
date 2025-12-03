#pragma once

#include "model.hpp"

namespace wibot {

/**
 * @brief 线性映射管道
 * 
 * 将int16_t输入范围线性映射到float输出范围。支持多通道实时无状态映射。
 * 所有通道共享同一套映射配置。
 * 公式：output = (input - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin
 * 
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <u8 CHANNELS>
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
     * @brief 构造线性映射器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的映射配置
     */
    LinearMapper(SyncPipeline<i16>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
    }

    /**
     * @brief 获取映射后的值
     */
    f32 getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return 0.0f;  // 无效通道返回0
        }

        // 获取上游值并实时处理
        i16 input = _upstream.getValue(channel);
        return _mapLinear(static_cast<f32>(input));
    }

    /**
     * @brief 重置管道状态
     */
    void reset() override {
        // 无状态映射器，无需重置，但需要重置上游
        _upstream.reset();
    }

    /**
     * @brief 更新管道状态
     */
    void update() override {
        // 无状态映射器，只需要更新上游
        _upstream.update();
    }

    /**
     * @brief 更新映射配置（影响所有通道）
     */
    void updateConfig(const Config& config) {
        _config = config;
    }

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
    Config             _config;    ///< 共享的映射配置
};

}  // namespace wibot
