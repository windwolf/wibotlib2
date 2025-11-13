#pragma once

#include "model.hpp"

namespace wibot {

/**
 * @brief 分桶区间配置结构
 */
struct BinRange {
    f32 lowerBound;  ///< 区间下界
    f32 upperBound;  ///< 区间上界

    BinRange() : lowerBound(0.0f), upperBound(0.0f) {
    }
    BinRange(f32 lower, f32 upper) : lowerBound(lower), upperBound(upper) {
    }
};

/**
 * @brief 分桶映射管道
 * 
 * 将连续数值映射到离散区间，支持多通道实时处理。
 * 相邻区间设有滞回区域，防止边界数值频繁跳动。
 * 所有通道共享同一套区间配置。
 * 
 * 区间分布：
 * [bin0] [hysteresis] [bin1] [hysteresis] [bin2] ... [binN-1]
 * 
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <u8 CHANNELS>
class BinningMapper : public SyncPipeline<u32> {
   public:
    /**
     * @brief 分桶映射配置
     */
    struct Config {
        /**
         * @brief 区间定义数组
         * 
         * 区间必须按照从小到大的顺序排列，且不能重叠。
         * 相邻区间之间的滞回区域由 hysteresisWidth 定义。
         */
        BinRange* ranges;

        /**
         * @brief 区间数量
         */
        u32 binCount;

        /**
         * @brief 滞回区域宽度
         * 
         * 在相邻区间边界处添加的滞回宽度，防止边界值频繁跳动。
         * 滞回区域 = [边界 - hysteresisWidth/2, 边界 + hysteresisWidth/2]
         */
        f32 hysteresisWidth;

        /**
         * @brief 是否启用滞回功能
         */
        bool enableHysteresis;

        /**
         * @brief 超出范围时的处理方式
         * true: 钳制到边界区间 (返回0或binCount-1)
         * false: 返回特殊值 (如UINT32_MAX表示无效)
         */
        bool clampToRange;

        Config()
            : ranges(nullptr),
              binCount(0),
              hysteresisWidth(0.0f),
              enableHysteresis(true),
              clampToRange(true) {
        }
    };

    /**
     * @brief 无效区间索引标记
     */
    static constexpr u32 INVALID_BIN_INDEX = UINT32_MAX;

   public:
    /**
     * @brief 构造分桶映射器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的分桶配置
     */
    BinningMapper(SyncPipeline<f32>& upstream, const Config& config);

    /**
     * @brief 析构函数，清理资源
     */
    ~BinningMapper();

    /**
     * @brief 获取分桶映射后的区间索引
     */
    u32 getValue(u8 channel) const override;

    /**
     * @brief 重置管道状态
     */
    void reset() override;

    /**
     * @brief 更新管道状态
     */
    void update() override;

    /**
     * @brief 更新分桶配置（影响所有通道）
     */
    void updateConfig(const Config& config);

    /**
     * @brief 获取通道数量
     */
    u8 getChannelCount() const {
        return CHANNELS;
    }

   private:
    /**
     * @brief 执行分桶映射（不带滞回）
     */
    u32 _mapToBinSimple(f32 value) const;

    /**
     * @brief 执行分桶映射（带滞回）
     * @param value 输入值
     * @param channel 通道索引
     * @return 区间索引
     */
    u32 _mapToBinWithHysteresis(f32 value, u8 channel) const;

   private:
    SyncPipeline<f32>& _upstream;                ///< 上游管道引用
    Config             _config;                  ///< 共享的分桶配置
    mutable u32        _lastBinIndex[CHANNELS];  ///< 每通道的上次区间索引（用于滞回判断）
};

}  // namespace wibot
