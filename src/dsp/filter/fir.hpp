#pragma once

#include "type.hpp"

#if defined(CMSIS_DSP_ENABLED)

#include "arm_math.h"

#endif

namespace wibot {

/**
 * @brief 线性相位 FIR 滤波器
 *
 * 滤波系数由外部提供（使用 FIRDesigner 设计）
 * 
 * 使用示例：
 * @code
 * // 编译期设计（推荐）
 * constexpr f32 lpf_coeffs[21] = {};
 * FIRDesigner::designLowpassConstexpr(lpf_coeffs, 20000.0f, 1000.0f);
 * FIR::Config cfg = {.coeffs = lpf_coeffs, .tapCount = 21};
 * 
 * // 运行时设计
 * f32 coeffs[21];
 * FIRDesigner::designLowpass(coeffs, {20000.0f, 1000.0f, 21, WindowType::Hamming});
 * FIR::Config cfg = {.coeffs = coeffs, .tapCount = 21};
 * @endcode
 */
class FIR {
   public:
    struct Config {
        const f32* coeffs;    // 滤波器系数数组（必须提供）
        u16        tapCount;  // 滤波器抽头数量
    };

    explicit FIR(const Config& cfg);

    bool applyConfig();

    f32 filter(f32 input);

    void reset();

    static bool isConfigValid(const Config& cfg);

   private:
    static constexpr u16 MAX_TAPS = 64;

   private:
    const Config& _config;
    f32           _coeffs[MAX_TAPS]{};
    u16           _tapCount{0};

#if defined(CMSIS_DSP_ENABLED)
    arm_fir_instance_f32 _firInstance{};             // CMSIS-DSP FIR实例
    f32                  _firState[MAX_TAPS + 1]{};  // 状态缓冲区 (numTaps + 2 * blockSize - 1)
#else
    f32 _xbuf[MAX_TAPS]{};  // 自定义实现的样本缓冲区
    u16 _idx{0};            // 循环缓冲区索引
#endif
};

}  // namespace wibot
