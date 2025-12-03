#pragma once

#include "../model.hpp"

namespace wibot {

/**
 * @brief 中值滤波器管道
 * 
 * 将float输入值进行中值滤波处理。支持多通道实时状态滤波。
 * 所有通道共享同一套滤波配置，但各通道维护独立的滤波状态。
 * 
 * 中值滤波器通过维护一个滑动窗口，计算窗口内数据的中值：
 * - 对于奇数窗口大小，返回排序后中间位置的值
 * - 对于偶数窗口大小，返回排序后中间两个值的平均值
 * 
 * 中值滤波器特别适用于去除脉冲噪声，能够很好地保持信号的边缘特性。
 * 
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <u8 CHANNELS>
class MedianFilter : public SyncPipeline<f32> {
   public:
    /**
     * @brief 中值滤波器配置
     */
    struct Config {
        u8 windowSize;     ///< 滤波窗口大小，建议使用奇数
        u8 maxWindowSize;  ///< 缓冲区最大窗口大小（用于验证）
    };

   public:
    /**
     * @brief 构造中值滤波器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的滤波配置
     * @param buffers 外部提供的缓冲区指针数组[CHANNELS]，每个指向[maxWindowSize]的数组
     * @param tempBuffer 外部提供的临时缓冲区[maxWindowSize]
     */
    MedianFilter(SyncPipeline<f32>& upstream, const Config& config, f32* buffers[CHANNELS],
                 f32* tempBuffer);

    /**
     * @brief 析构函数（无需手动内存管理）
     */
    ~MedianFilter() = default;

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
     * @note 更新配置会重置所有通道状态（无内存重分配）
     */
    void updateConfig(const Config& config);

    /**
     * @brief 验证配置是否有效
     * 
     * @param config 要验证的配置
     * @return true 配置有效
     * @return false 配置无效（窗口大小为0或超出最大窗口大小等）
     */
    static bool isConfigValid(const Config& config);

   private:
    /**
     * @brief 对单个通道的缓冲区进行中值计算
     * 
     * @param channel 通道索引
     * @return f32 中值滤波后的输出值
     */
    f32 _calculateMedian(u8 channel);

    /**
     * @brief 初始化缓冲区状态
     */
    void _initializeBuffers();

    /**
     * @brief 快速选择算法计算中值（避免完整排序）
     * 
     * @param arr 数据数组
     * @param left 起始索引
     * @param right 结束索引
     * @param k 目标位置
     * @return f32 第k小的元素
     */
    f32 _quickSelect(f32* arr, int left, int right, int k);

    /**
     * @brief 数组分区函数（快速选择算法辅助函数）
     */
    int _partition(f32* arr, int left, int right);

   private:
    SyncPipeline<f32>& _upstream;  ///< 上游管道引用
    Config             _config;    ///< 共享的滤波配置

    // 各通道的滤波状态（外部缓冲区）
    f32* _buffers[CHANNELS];      ///< 各通道的环形缓冲区指针（外部提供）
    u8   _bufferIndex[CHANNELS];  ///< 各通道的缓冲区当前索引
    u8   _bufferCount[CHANNELS];  ///< 各通道的缓冲区有效数据数量
    f32  _outputLast[CHANNELS];   ///< 各通道上次的输出值
    f32* _tempBuffer;             ///< 临时缓冲区指针（外部提供）
};

}  // namespace wibot
