#pragma once

#include "model.hpp"

namespace wibot {

/**
 * @brief 分段线性映射管道
 * 
 * 将int16_t输入按照分段线性函数映射到float输出。支持多通道实时无状态映射。
 * 所有通道共享同一套分段映射配置。
 * 
 * 分段线性映射通过多个控制点定义，每两个相邻控制点之间进行线性插值。
 * 超出范围的输入可以选择钳位到边界值或进行外推。
 * 
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <u8 CHANNELS>
class PiecewiseLinearMapper : public SyncPipeline<f32> {
   public:
    /**
     * @brief 分段线性映射配置
     */
    struct Config {
        const f32* inputPoints;          ///< 输入控制点数组指针（必须按升序排列）
        const f32* outputPoints;         ///< 输出控制点数组指针
        u8         segmentCount;         ///< 分段数量（控制点数量为segmentCount+1）
        bool       clampOutput;          ///< 是否将输出限制在边界值范围内
        bool       enableExtrapolation;  ///< 是否允许超出范围时进行外推
    };

   public:
    /**
     * @brief 构造分段线性映射器（所有通道使用相同配置）
     * 
     * @param upstream 上游管道
     * @param config 共享的分段映射配置
     */
    PiecewiseLinearMapper(SyncPipeline<i16>& upstream, const Config& config);

    /**
     * @brief 获取映射后的值
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
     * @brief 更新映射配置（影响所有通道）
     * 
     * @param config 新的分段映射配置
     * @note 输入控制点必须按升序排列
     */
    void updateConfig(const Config& config);

    /**
     * @brief 验证配置是否有效
     * 
     * @param config 要验证的配置
     * @return true 配置有效
     * @return false 配置无效（控制点未按升序排列等）
     */
    static bool isConfigValid(const Config& config);

   private:
    /**
     * @brief 执行分段线性映射
     * 
     * @param input 输入值
     * @return f32 映射后的输出值
     */
    f32 _mapPiecewiseLinear(f32 input) const;

    /**
     * @brief 查找输入值所在的分段索引
     * 
     * @param input 输入值
     * @return u8 分段索引（0到segmentCount-1），如果超出范围返回特殊值
     */
    u8 _findSegmentIndex(f32 input) const;

    /**
     * @brief 在指定分段内进行线性插值
     * 
     * @param input 输入值
     * @param segmentIndex 分段索引
     * @return f32 插值结果
     */
    f32 _interpolateInSegment(f32 input, u8 segmentIndex) const;

   private:
    SyncPipeline<i16>& _upstream;  ///< 上游管道引用
    Config             _config;    ///< 共享的分段映射配置

    // 常量定义
    static constexpr u8 INVALID_SEGMENT = 0xFF;  ///< 无效分段标识
};

}  // namespace wibot
