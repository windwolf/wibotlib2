#pragma once

#include "../model.hpp"
#include <functional>

namespace wibot {
/**
     * @brief 映射函数类型定义
     * 
     * @param input 输入值
     * @param channel 通道索引（可选用于通道相关的映射）
     * @return TOut 映射后的输出值
 */
template <typename TIn, typename TOut>
using MappingFunction = std::function<TOut(TIn input, u8 channel)>;
/**
 * @brief 自定义映射管道
 * 
 * 提供用户自定义映射函数的映射器。支持多通道实时无状态映射。
 * 用户可以提供自定义的映射函数来实现任意复杂的映射逻辑。
 * 所有通道共享同一个映射函数和配置。
 * 
 * @tparam TIn 输入数据类型
 * @tparam TOut 输出数据类型
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <typename TIn, typename TOut, u8 CHANNELS>
class CustomMapper : public SyncPipeline<TOut> {
   public:
    /**
     * @brief 自定义映射配置
     */
    struct Config {
        MappingFunction<TIn, TOut> mappingFunc;  ///< 用户提供的映射函数
    };

   public:
    /**
     * @brief 构造自定义映射器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的映射配置
     */
    CustomMapper(SyncPipeline<TIn>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
        // 验证配置有效性
        if (!isConfigValid(config)) {
            // 如果配置无效，提供一个默认的恒等映射函数
            _config.mappingFunc = [](TIn input, u8 /*channel*/) -> TOut {
                return static_cast<TOut>(input);
            };
        }
    }

    /**
     * @brief 获取映射后的值
     */
    TOut getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return TOut{};  // 无效通道返回默认值
        }

        // 获取上游值并直接调用映射函数
        TIn input = _upstream.getValue(channel);
        return _config.mappingFunc(input, channel);
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
        if (isConfigValid(config)) {
            _config = config;
        }
        // 如果配置无效，保持原有配置不变
    }

    /**
     * @brief 验证配置是否有效
     * 
     * @param config 要验证的配置
     * @return true 配置有效
     * @return false 配置无效（映射函数为空等）
     */
    static bool isConfigValid(const Config& config) {
        // 检查映射函数是否有效
        return static_cast<bool>(config.mappingFunc);
    }

   private:
   private:
    SyncPipeline<TIn>& _upstream;  ///< 上游管道引用
    Config             _config;    ///< 共享的映射配置
};

}  // namespace wibot
