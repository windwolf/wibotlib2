#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 线性映射
 * 
 */
class LinearMapper {
   public:
    struct Config {
        f32  inputMin;     // 输入最小值
        f32  inputMax;     // 输入最大值
        f32  outputMin;    // 输出最小值
        f32  outputMax;    // 输出最大值
        bool clampOutput;  // 是否限制输出范围
    };

    /**
     * @brief 构造函数
     * @param cfg 映射配置
     */
    explicit LinearMapper(const Config& cfg);

    /**
     * @brief 处理单个样本
     * @return 映射后的值
     */
    f32 process(f32 input);

    /**
     * @brief 验证配置有效性
     */
    static bool isConfigValid(const Config& cfg);

   private:
    const Config& _config;
};

} // namespace wibot

