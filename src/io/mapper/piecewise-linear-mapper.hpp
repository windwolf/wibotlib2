#pragma once

#include "model.hpp"
#include <cmath>

namespace wibot {

/**
 * @brief 分段线性映射管道
 * 
 * 将int16_t输入按照分段线性函数映射到float输出。
 * 
 * 分段线性映射通过多个控制点定义，每两个相邻控制点之间进行线性插值。
 * 超出范围的输入可以选择钳位到边界值或进行外推。
 * 
 * 配置使用引用方式，支持多个映射器实例共享同一配置。
 */
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
     * @brief 构造分段线性映射器
     * 
     * @param upstream 上游管道
     * @param config 分段映射配置（引用方式，支持共享）
     */
    PiecewiseLinearMapper(SyncPipeline<i16>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
        // 构造时验证配置有效性
        if (!isConfigValid(config)) {
            // 如果配置无效，可以选择抛出异常或使用默认配置
            // 这里暂时保留原配置，实际使用时需要注意验证
        }
    }

    f32 getValue() const override {
        // 获取上游值并实时处理
        i16 input = _upstream.getValue();
        return _mapPiecewiseLinear(static_cast<f32>(input));
    }

    void reset() override {
        _upstream.reset();
    }

    void update() override {
        _upstream.update();
    }

    /**
     * @brief 验证配置是否有效
     * 
     * @param config 要验证的配置
     * @return true 配置有效
     * @return false 配置无效（控制点未按升序排列等）
     */
    static bool isConfigValid(const Config& config) {
        // 检查基本参数
        if (config.inputPoints == nullptr || config.outputPoints == nullptr ||
            config.segmentCount == 0) {
            return false;
        }

        // 检查输入控制点是否按升序排列
        for (u8 i = 0; i < config.segmentCount; ++i) {
            if (config.inputPoints[i] >= config.inputPoints[i + 1]) {
                return false;  // 控制点未按升序排列
            }
        }
        return true;
    }

   private:
    /**
     * @brief 执行分段线性映射
     * 
     * @param input 输入值
     * @return f32 映射后的输出值
     */
    f32 _mapPiecewiseLinear(f32 input) const {
        // 查找输入值所在的分段
        u8 segmentIndex = _findSegmentIndex(input);

        if (segmentIndex == INVALID_SEGMENT) {
            // 输入值超出所有分段范围
            if (_config.enableExtrapolation) {
                // 进行外推
                if (input < _config.inputPoints[0]) {
                    // 在第一个分段之前，使用第一个分段的斜率外推
                    return _interpolateInSegment(input, 0);
                } else {
                    // 在最后一个分段之后，使用最后一个分段的斜率外推
                    return _interpolateInSegment(input, _config.segmentCount - 1);
                }
            } else {
                // 不允许外推，返回边界值
                if (input < _config.inputPoints[0]) {
                    return _config.outputPoints[0];
                } else {
                    return _config.outputPoints[_config.segmentCount];
                }
            }
        }

        // 在有效分段内进行插值
        f32 result = _interpolateInSegment(input, segmentIndex);

        // 如果启用了输出鑷位，限制结果范围
        if (_config.clampOutput) {
            f32 minOutput = _config.outputPoints[0];
            f32 maxOutput = _config.outputPoints[0];

            // 找到输出范围
            for (u8 i = 1; i <= _config.segmentCount; ++i) {
                if (_config.outputPoints[i] < minOutput) {
                    minOutput = _config.outputPoints[i];
                }
                if (_config.outputPoints[i] > maxOutput) {
                    maxOutput = _config.outputPoints[i];
                }
            }

            if (result < minOutput) result = minOutput;
            if (result > maxOutput) result = maxOutput;
        }

        return result;
    }

    /**
     * @brief 查找输入值所在的分段索引
     * 
     * @param input 输入值
     * @return u8 分段索引（0到segmentCount-1），如果超出范围返回特殊值
     */
    u8 _findSegmentIndex(f32 input) const {
        // 检查是否在范围内
        if (input < _config.inputPoints[0] || input > _config.inputPoints[_config.segmentCount]) {
            return INVALID_SEGMENT;
        }

        // 查找输入值所在的分段
        for (u8 i = 0; i < _config.segmentCount; ++i) {
            if (input >= _config.inputPoints[i] && input <= _config.inputPoints[i + 1]) {
                return i;
            }
        }

        return INVALID_SEGMENT;
    }

    /**
     * @brief 在指定分段内进行线性插值
     * 
     * @param input 输入值
     * @param segmentIndex 分段索引
     * @return f32 插值结果
     */
    f32 _interpolateInSegment(f32 input, u8 segmentIndex) const {
        // 确保分段索引有效
        if (segmentIndex >= _config.segmentCount) {
            return NAN;  // 无效分段
        }

        f32 x0 = _config.inputPoints[segmentIndex];
        f32 x1 = _config.inputPoints[segmentIndex + 1];
        f32 y0 = _config.outputPoints[segmentIndex];
        f32 y1 = _config.outputPoints[segmentIndex + 1];

        // 防止除零错误
        if (x1 == x0) {
            return y0;  // 如果输入点相同，返回第一个输出值
        }

        // 线性插值公式：y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
        f32 ratio = (input - x0) / (x1 - x0);
        return y0 + (y1 - y0) * ratio;
    }

   private:
    SyncPipeline<i16>& _upstream;  ///< 上游管道引用
    const Config&      _config;    ///< 分段映射配置引用（支持共享）

    // 常量定义
    static constexpr u8 INVALID_SEGMENT = 0xFF;  ///< 无效分段标识
};

}  // namespace wibot
