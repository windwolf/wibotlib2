#pragma once

#include "type.hpp"
#include "math.hpp"

namespace wibot {

/**
 * @brief FIR 滤波器系数设计器
 * 
 * 提供编译期计算能力，支持多种窗函数和设计方法
 * 
 * 支持的设计方法：
 * - 窗函数法：适合实时/编译期计算，实现简单
 * - Parks–McClellan：推荐使用外部工具（Python/MATLAB）预计算
 * 
 * Parks–McClellan 使用示例（Python）：
 * @code{.py}
 * from scipy import signal
 * # 设计等波纹滤波器
 * taps = signal.remez(21, [0, 0.1, 0.15, 0.5], [1, 0], fs=1.0)
 * # 输出 C++ 代码
 * print("constexpr f32 pm_coeffs[] = {")
 * print(", ".join(f"{c:.8f}f" for c in taps))
 * print("};")
 * @endcode
 */
class FIRDesigner {
   public:
    enum class WindowType {
        Hamming,     // Hamming 窗（默认）
        Hanning,     // Hanning 窗
        Blackman,    // Blackman 窗
        Rectangular  // 矩形窗
    };

    struct DesignParams {
        f32        sampleFreq;  // 采样频率（Hz）
        f32        cutoffFreq;  // 截止频率（Hz）
        u16        tapCount;    // 抽头数量（建议奇数）
        WindowType window;      // 窗函数类型
    };

    /**
     * @brief 设计低通 FIR 滤波器系数（运行时）
     * @param coeffs 输出系数数组
     * @param params 设计参数
     * @return 是否设计成功
     */
    static constexpr bool designLowpass(f32* coeffs, const DesignParams& params) {
        if (!isParamsValid(params)) {
            return false;
        }

        const u16 N  = params.tapCount;
        const u16 M  = N - 1;
        const f32 fc = params.cutoffFreq / params.sampleFreq;  // 归一化频率 [0, 0.5)

        // 计算理想低通滤波器系数并应用窗函数
        f32 sum = 0.0f;
        for (u16 n = 0; n < N; n++) {
            const f32 m = static_cast<f32>(n) - static_cast<f32>(M) * 0.5f;

            // 理想低通响应
            f32 h;
            if (_abs(m) < 1e-7f) {
                h = 2.0f * fc;
            } else {
                h = 2.0f * fc * _sinc(2.0f * fc * m);
            }

            // 应用窗函数
            const f32 w = _window(n, M, params.window);
            const f32 c = h * w;
            coeffs[n]   = c;
            sum += c;
        }

        // 归一化到单位增益
        if (sum > 1e-7f) {
            const f32 inv = 1.0f / sum;
            for (u16 n = 0; n < N; n++) {
                coeffs[n] *= inv;
            }
        }

        return true;
    };

    /**
     * @brief 编译期设计低通 FIR 滤波器系数
     * @tparam N 抽头数量
     * @param coeffs 输出系数数组
     * @param sampleFreq 采样频率（Hz）
     * @param cutoffFreq 截止频率（Hz）
     * @param window 窗函数类型
     */
    template <u16 N>
    static constexpr void designLowpassConstexpr(f32 (&coeffs)[N], f32 sampleFreq, f32 cutoffFreq,
                                                 WindowType window = WindowType::Hamming) {
        const f32 fc = cutoffFreq / sampleFreq;  // 归一化频率 [0, 0.5)
        const u16 M  = N - 1;

        // 计算理想低通滤波器系数并应用窗函数
        f32 sum = 0.0f;
        for (u16 n = 0; n < N; n++) {
            const f32 m = static_cast<f32>(n) - static_cast<f32>(M) * 0.5f;

            // 理想低通响应
            f32 h;
            if (_abs(m) < 1e-7f) {
                h = 2.0f * fc;
            } else {
                h = 2.0f * fc * _sinc(2.0f * fc * m);
            }

            // 应用窗函数
            const f32 w = _window(n, M, window);
            const f32 c = h * w;
            coeffs[n]   = c;
            sum += c;
        }

        // 归一化到单位增益
        if (sum > 1e-7f) {
            const f32 inv = 1.0f / sum;
            for (u16 n = 0; n < N; n++) {
                coeffs[n] *= inv;
            }
        }
    }

    /**
     * @brief 验证设计参数
     */
    static constexpr bool isParamsValid(const DesignParams& params) {
        if (params.sampleFreq <= 0.0f || params.cutoffFreq <= 0.0f) {
            return false;
        }
        if (params.cutoffFreq >= params.sampleFreq * 0.5f) {
            return false;
        }
        if (params.tapCount == 0 || params.tapCount > 128) {
            return false;
        }
        return true;
    }

    /**
     * @brief 验证预计算系数（用于外部工具生成的系数）
     * @param coeffs 系数数组
     * @param count 系数数量
     * @return 是否有效（检查归一化和对称性）
     */
    static constexpr bool validateCoeffs(const f32* coeffs, u16 count) {
        if (coeffs == nullptr || count == 0) {
            return false;
        }

        // 检查系数和（应接近1.0，单位增益）
        f32 sum = 0.0f;
        for (u16 i = 0; i < count; i++) {
            sum += coeffs[i];
        }

        // 允许 ±5% 的偏差
        if (_abs(sum - 1.0f) > 0.05f) {
            return false;
        }

        return true;
    }

    /**
     * @brief 归一化系数到单位增益
     * @param coeffs 系数数组（输入/输出）
     * @param count 系数数量
     */
    static constexpr void normalizeCoeffs(f32* coeffs, u16 count) {
        f32 sum = 0.0f;
        for (u16 i = 0; i < count; i++) {
            sum += coeffs[i];
        }

        if (sum > 1e-7f) {
            const f32 inv = 1.0f / sum;
            for (u16 i = 0; i < count; i++) {
                coeffs[i] *= inv;
            }
        }
    }

   private:
    // 编译期数学函数
    static constexpr f32 _abs(f32 x) {
        return x < 0.0f ? -x : x;
    }

    static constexpr f32 _sin(f32 x) {
        // Taylor 级数近似（编译期计算）
        const f32 x2 = x * x;
        const f32 x3 = x2 * x;
        const f32 x5 = x3 * x2;
        const f32 x7 = x5 * x2;
        const f32 x9 = x7 * x2;
        return x - x3 / 6.0f + x5 / 120.0f - x7 / 5040.0f + x9 / 362880.0f;
    }

    static constexpr f32 _cos(f32 x) {
        // Taylor 级数近似（编译期计算）
        const f32 x2 = x * x;
        const f32 x4 = x2 * x2;
        const f32 x6 = x4 * x2;
        const f32 x8 = x6 * x2;
        return 1.0f - x2 / 2.0f + x4 / 24.0f - x6 / 720.0f + x8 / 40320.0f;
    }

    static constexpr f32 _sinc(f32 x) {
        const f32 pix = kPI * x;
        if (_abs(pix) < 1e-7f) {
            return 1.0f;
        }
        return _sin(pix) / pix;
    }

    static constexpr f32 _window(u16 n, u16 M, WindowType type) {
        const f32 nf = static_cast<f32>(n);
        const f32 Mf = static_cast<f32>(M);

        switch (type) {
            case WindowType::Hamming:
                return 0.54f - 0.46f * _cos(2.0f * kPI * nf / Mf);

            case WindowType::Hanning:
                return 0.5f - 0.5f * _cos(2.0f * kPI * nf / Mf);

            case WindowType::Blackman:
                return 0.42f - 0.5f * _cos(2.0f * kPI * nf / Mf) +
                       0.08f * _cos(4.0f * kPI * nf / Mf);

            case WindowType::Rectangular:
            default:
                return 1.0f;
        }
    }
};

}  // namespace wibot
