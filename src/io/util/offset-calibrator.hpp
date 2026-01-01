#pragma once

#include "type.hpp"
#include <cstdint>

namespace wibot {

/**
 * @brief 偏移校准器
 * 
 * 提供数据采集、累计和平均计算功能，用于自动校准ADC的偏移量。
 * 用户代码需要手动控制采样时机，校准器仅负责数据处理。
 * 
 * 特性：
 * - 用户控制采样时机和数量
 * - 高精度累计器避免溢出
 */
class OffsetCalibrator {
   public:
    struct Storage {
        u16 currentSampleCount{0};  ///< 当前样本数
        u32 accumulator{0};         ///< 累加器
        i16 offset{0};              ///< 计算得出的偏移量
    };

    /**
     * @brief 构造校准器
     */
    explicit OffsetCalibrator(Storage& storage);

    /**
     * @brief 重置累计器和计数器
     * 
     * 清零所有累计器和计数器，准备收集新的样本
     */
    void reset();

    /**
     * @brief 添加样本数据
     * 
     * @param value 原始ADC值
     */
    void addSample(u16 value);

    /**
     * @brief 计算校准结果
     * 
     * 根据当前累积的样本计算平均偏移量
     * 
     * @return true 计算成功，false 没有有效样本
     */
    bool calculate();

    /**
     * @brief 获取校准偏移量
     * 
     * @return i16 校准偏移量
     */
    i16 getOffset() const;

    /**
     * @brief 获取当前样本数量
     * 
     * @return u16 已采集的样本数
     */
    u16 getSampleCount() const;

   private:
    Storage& _storage;
};

}  // namespace wibot