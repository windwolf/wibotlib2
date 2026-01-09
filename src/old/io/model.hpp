#pragma once

#include "async.hpp"
#include "type.hpp"

namespace wibot {

/**
 * @brief 数据处理管道的基础接口（单通道）
 * 
 * SyncPipeline 提供了流式数据处理的核心抽象，支持管道嵌套和链式组合。
 * 所有数据源、滤波器、映射器、分桶器等都实现此接口。
 * 
 * 简化设计：每个 Pipeline 实例处理单个数据流。
 * 多通道场景通过创建多个 Pipeline 实例或使用专门的多通道源组件实现。
 * 
 * @tparam T 管道处理的数据类型
 */
template <typename T>
class SyncPipeline {
   public:
    /**
     * @brief 更新管道状态
     * 
     * 触发数据的流式处理，从上游获取数据并进行处理。
     * 对于数据源管道，通常从硬件或缓存中读取数据。
     * 对于处理管道，从上游管道获取数据并应用处理逻辑。
     */
    virtual void update() = 0;

    /**
     * @brief 获取管道输出值
     * 
     * @return T 处理后的输出值
     */
    virtual T getValue() const = 0;

    /**
     * @brief 重置管道状态
     * 
     * 清除所有内部缓存和历史状态，将管道重置到初始状态。
     */
    virtual void reset() = 0;

    virtual ~SyncPipeline() = default;
};

/**
 * @brief 多通道数据处理管道接口
 * 
 * MultiChannelPipeline 用于硬件层面天然支持多通道的组件，如ADC、GPIO、按键扫描器。
 * 提供批量更新和按通道访问的能力。
 * 
 * @tparam T 管道处理的数据类型
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <typename T, u8 CHANNELS>
class MultiChannelPipeline {
   public:
    /**
     * @brief 更新所有通道的状态
     */
    virtual void update() = 0;

    /**
     * @brief 获取指定通道的输出值
     * 
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @return T 该通道处理后的输出值
     */
    virtual T getValue(u8 channel) const = 0;

    /**
     * @brief 重置所有通道的状态
     */
    virtual void reset() = 0;

    /**
     * @brief 获取通道数量
     */
    static constexpr u8 getChannelCount() {
        return CHANNELS;
    }

    virtual ~MultiChannelPipeline() = default;
};



}  // namespace wibot