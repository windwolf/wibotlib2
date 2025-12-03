#pragma once

#include "../model.hpp"
#include <functional>

namespace wibot {
/**
 * @brief 映射函数类型定义
 * 
 * @param input 输入值
 * @return TOut 映射后的输出值
 */
template <typename TIn, typename TOut>
using MappingFunction = std::function<TOut(TIn input)>;
/**
 * @brief 自定义映射管道
 * 
 * 提供用户自定义映射函数的映射器。
 * 用户可以提供自定义的映射函数来实现任意复杂的映射逻辑。
 * 
 * 配置使用引用方式，支持多个映射器实例共享同一配置。
 * 
 * @tparam TIn 输入数据类型
 * @tparam TOut 输出数据类型
 */
template <typename TIn, typename TOut>
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
     * @brief 构造自定义映射器
     * 
     * @param upstream 上游管道
     * @param config 映射配置（引用方式，支持共享）
     */
    CustomMapper(SyncPipeline<TIn>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
        // 验证配置有效性
        if (!isConfigValid(config)) {
            // 如果配置无效，提供一个默认的恒等映射函数
            static const Config defaultConfig{
                [](TIn input) -> TOut { return static_cast<TOut>(input); }};
            _config = defaultConfig;
        }
    }

    TOut getValue() const override {
        // 获取上游值并直接调用映射函数
        TIn input = _upstream.getValue();
        return _config.mappingFunc(input);
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
    void updateConfig(const Config& config) {
        if (isConfigValid(config)) {
            _config = config;
        }
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
    SyncPipeline<TIn>& _upstream;  ///< 上游管道引用
    const Config&      _config;    ///< 映射配置引用（支持共享）
};

}  // namespace wibot
