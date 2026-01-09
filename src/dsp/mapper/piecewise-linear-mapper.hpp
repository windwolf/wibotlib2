#pragma once

#include "type.hpp"

namespace wibot::dsp {

class PiecewiseLinearMapper {
   public:
    static constexpr u8 INVALID_SEGMENT = 0xFF;

    struct Config {
        const f32* inputPoints{nullptr};        // 输入控制点数组（必须按升序排列）
        const f32* outputPoints{nullptr};       // 输出控制点数组
        u8         segmentCount{0};             // 分段数量（控制点数量为segmentCount+1）
        bool       clampOutput{false};          // 是否将输出限制在边界值范围内
        bool       enableExtrapolation{false};  // 是否允许超出范围时进行外推
    };

   public:
    explicit PiecewiseLinearMapper(Config& config);

    f32 map(f32 input);

    static bool isConfigValid(const Config& config);

   private:
    u8 findSegmentIndex(f32 input) const;

    f32 interpolateInSegment(f32 input, u8 segmentIndex) const;

   private:
    Config& _config;
};

}  // namespace wibot::dsp
