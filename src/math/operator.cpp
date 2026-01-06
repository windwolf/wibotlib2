
#include "operator.hpp"

#include <algorithm>

#include "cordic.hpp"

#if defined(HAL_CORDIC_MODULE_ENABLED)
extern "C" {
#include "cordic.h"
}
#endif

#if defined(CMSIS_DSP_ENABLED)
extern "C" {
#include "arm_math.h"
}
#endif

namespace wibot {

Math::Math() : _cordic(nullptr) {
}
#ifdef HAL_CORDIC_MODULE_ENABLED
Math::Math(Cordic& hcordic) : _cordic(&hcordic) {
}
#endif

template <typename T>
constexpr T clampValue(T v, T lo, T hi) {
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

inline q15 floatToQ15(f32 v) {
    return static_cast<q15>(clampValue<f32>(v, -0.9999695f, 0.9999695f) * kQ15Scale);
}

inline q31 floatToQ31(f32 v) {
    return static_cast<q31>(clampValue<f32>(v, -0.999999999f, 0.999999999f) * kQ31Scale);
}

inline f32 q15ToFloat(q15 v) {
    return static_cast<f32>(v) / kQ15Scale;
}

inline f32 q31ToFloat(q31 v) {
    return static_cast<f32>(v) / kQ31Scale;
}

template <typename T, typename Acc>
constexpr T saturateAdd(T a, T b, Acc minv, Acc maxv) {
    Acc s = static_cast<Acc>(a) + static_cast<Acc>(b);
    if (s > maxv) {
        s = maxv;
    } else if (s < minv) {
        s = minv;
    }
    return static_cast<T>(s);
}

template <typename T, typename Acc>
constexpr T saturateShiftMul(T a, T b, Acc shift, Acc minv, Acc maxv) {
    Acc p = static_cast<Acc>(a) * static_cast<Acc>(b);
    p += static_cast<Acc>(1) << (shift - 1);  // rounding
    p >>= shift;
    if (p > maxv) {
        p = maxv;
    } else if (p < minv) {
        p = minv;
    }
    return static_cast<T>(p);
}

// 基础运算

template <>
q15 Math::add<q15>(q15 a, q15 b) {
#if defined(CMSIS_DSP_ENABLED)
    q15 out;
    arm_add_q15(&a, &b, &out, 1);
    return out;
#else
    return saturateAdd<q15, int32_t>(a, b, INT16_MIN, INT16_MAX);
#endif
};

template <>
q31 Math::add<q31>(q31 a, q31 b) {
#if defined(CMSIS_DSP_ENABLED)
    q31 out;
    arm_add_q31(&a, &b, &out, 1);
    return out;
#else
    return saturateAdd<q31, int64_t>(a, b, INT32_MIN, INT32_MAX);
#endif
};

template <>
q15 Math::sub<q15>(q15 a, q15 b) {
#if defined(CMSIS_DSP_ENABLED)
    q15 out;
    arm_sub_q15(&a, &b, &out, 1);
    return out;
#else
    return saturateAdd<q15, int32_t>(a, static_cast<q15>(-b), INT16_MIN, INT16_MAX);
#endif
}

template <>
q31 Math::sub<q31>(q31 a, q31 b) {
#if defined(CMSIS_DSP_ENABLED)
    q31 out;
    arm_sub_q31(&a, &b, &out, 1);
    return out;
#else
    return saturateAdd<q31, int64_t>(a, static_cast<q31>(-b), INT32_MIN, INT32_MAX);
#endif
}

template <>
q15 Math::mul<q15>(q15 a, q15 b) {
#if defined(CMSIS_DSP_ENABLED)
    q15 out;
    arm_mult_q15(&a, &b, &out, 1);
    return out;
#else
    return saturateShiftMul<q15, int32_t>(a, b, 15, INT16_MIN, INT16_MAX);
#endif
}

template <>
q31 Math::mul<q31>(q31 a, q31 b) {
#if defined(CMSIS_DSP_ENABLED)
    q31 out;
    arm_mult_q31(&a, &b, &out, 1);
    return out;
#else
    return saturateShiftMul<q31, int64_t>(a, b, 31, INT32_MIN, INT32_MAX);
#endif
}

// 三角函数

template <>
Vector2<f32> Math::sincos<f32>(f32 angle, f32 modulus) {
    f32 s;
    f32 c;
#if defined(CMSIS_DSP_ENABLED)
    arm_sin_cos_f32(angle, &s, &c);
#else
    s = std::sin(angle);
    c = std::cos(angle);
#endif
    return {s * modulus, c * modulus};
}

template <>
Vector2<q15> Math::sincos<q15>(q15 angle, q15 modulus) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sincos(angle, modulus);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    q15 s = arm_sin_q15(angle);
    q15 c = arm_cos_q15(angle);
    return {mul<q15>(s, modulus), mul<q15>(c, modulus)};
#else
    f32 s = std::sin(q15ToFloat(angle));
    f32 c = std::cos(q15ToFloat(angle));
    f32 m = q15ToFloat(modulus);
    return {floatToQ15(s * m), floatToQ15(c * m)};
#endif
}

template <>
Vector2<q31> Math::sincos<q31>(q31 angle, q31 modulus) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sincos(angle, modulus);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    q31 s;
    q31 c;
    arm_sin_cos_q31(angle, &s, &c);
    return {mul<q31>(s, modulus), mul<q31>(c, modulus)};
#else
    f32 s = std::sin(q31ToFloat(angle));
    f32 c = std::cos(q31ToFloat(angle));
    f32 m = q31ToFloat(modulus);
    return {floatToQ31(s * m), floatToQ31(c * m)};
#endif
}

template <>
Vector2f Math::phaseModulus<f32>(f32 x, f32 y) {
    return {std::atan2(y, x), std::sqrt(x * x + y * y)};
}

template <>
Vector2<q15> Math::phaseModulus<q15>(q15 x, q15 y) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->phaseModulus(x, y);
    }
#endif
    f32  xf = q15ToFloat(x);
    f32  yf = q15ToFloat(y);
    auto pm = phaseModulus<f32>(xf, yf);
    return {floatToQ15(pm.v1), floatToQ15(pm.v2)};
}

template <>
Vector2<q31> Math::phaseModulus<q31>(q31 x, q31 y) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->phaseModulus(x, y);
    }
#endif
    f32  xf = q31ToFloat(x);
    f32  yf = q31ToFloat(y);
    auto pm = phaseModulus<f32>(xf, yf);
    return {floatToQ31(pm.v1), floatToQ31(pm.v2)};
}

template <>
f32 Math::atan2<f32>(f32 x) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return q31ToFloat(_cordic->atan(static_cast<q31>(floatToQ31(x))));
    }
#endif
    return std::atan(x);
}

template <>
q15 Math::atan2<q15>(q15 x) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->atan(x);
    }
#endif
    return floatToQ15(std::atan(q15ToFloat(x)));
}

template <>
q31 Math::atan2<q31>(q31 x) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->atan(x);
    }
#endif
    return floatToQ31(std::atan(q31ToFloat(x)));
}

// 双曲线函数
template <>
Vector2f Math::sincosh<f32>(f32 angle) {
    return {std::sinh(angle), std::cosh(angle)};
}

template <>
Vector2<q15> Math::sincosh<q15>(q15 angle) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sincosh(angle);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point sincosh; fall back to float
    f32 a = q15ToFloat(angle);
    return {floatToQ15(std::sinh(a)), floatToQ15(std::cosh(a))};
#else
    f32 a = q15ToFloat(angle);
    return {floatToQ15(std::sinh(a)), floatToQ15(std::cosh(a))};
#endif
}

template <>
Vector2<q31> Math::sincosh<q31>(q31 angle) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sincosh(angle);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point sincosh; fall back to float
    f32 a = q31ToFloat(angle);
    return {floatToQ31(std::sinh(a)), floatToQ31(std::cosh(a))};
#else
    f32 a = q31ToFloat(angle);
    return {floatToQ31(std::sinh(a)), floatToQ31(std::cosh(a))};
#endif
}

template <>
f32 Math::atanh2<f32>(f32 x) {
    return std::atanh(x);
}

template <>
q15 Math::atanh2<q15>(q15 x) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->atanh(x);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point sincosh; fall back to float
    f32 xf = q15ToFloat(x);
    return floatToQ15(std::atanh(xf));
#else
    f32 xf = q15ToFloat(x);
    return floatToQ15(std::atanh(xf));
#endif
}

template <>
q31 Math::atanh2<q31>(q31 x) {
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->atanh(x);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point sincosh; fall back to float
    f32 xf = q31ToFloat(x);
    return floatToQ31(std::atanh(xf));
#else
    f32 xf = q31ToFloat(x);
    return floatToQ31(std::atanh(xf));
#endif
}

// 对数和平方根
template <>
f32 Math::log<f32>(f32 value) {
    return std::log(value);
}

template <>
q15 Math::log<q15>(q15 value) {
    if (value <= 0) {
        return 0;
    }
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->log(value);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point log; fall back to float
    f32 v = q15ToFloat(value);
    return floatToQ15(std::log(v));
#else
    f32 v = q15ToFloat(value);
    return floatToQ15(std::log(v));
#endif
}

template <>
q31 Math::log<q31>(q31 value) {
    if (value <= 0) {
        return 0;
    }
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->log(value);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    // CMSIS-DSP has no direct fixed-point log; fall back to float
    f32 v = std::max(0.0f, q31ToFloat(value));
    return floatToQ31(std::log(v));
#else
    f32 v = std::max(0.0f, q31ToFloat(value));
    return floatToQ31(std::log(v));
#endif
}

template <>
f32 Math::sqrt<f32>(f32 value) {
#if defined(CMSIS_DSP_ENABLED)
    f32        out   = 0.0f;
    arm_status state = arm_sqrt_f32(value, &out);
    return (state == ARM_MATH_SUCCESS) ? out : 0.0f;
#else
    return std::sqrt(value);
#endif
}

template <>
q15 Math::sqrt<q15>(q15 value) {
    if (value <= 0) {
        return 0;
    }
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sqrt(value);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    q15        out   = 0;
    arm_status state = arm_sqrt_q15(value, &out);
    return (state == ARM_MATH_SUCCESS) ? out : 0;
#else
    f32 v = q15ToFloat(value);
    return floatToQ15(std::sqrt(v));
#endif
}

template <>
q31 Math::sqrt<q31>(q31 value) {
    if (value <= 0) {
        return 0;
    }
#if defined(HAL_CORDIC_MODULE_ENABLED)
    if (_cordic != nullptr) {
        return _cordic->sqrt(value);
    }
#endif
#if defined(CMSIS_DSP_ENABLED)
    q31        out   = 0;
    arm_status state = arm_sqrt_q31(value, &out);
    return (state == ARM_MATH_SUCCESS) ? out : 0;
#else
    f32 v = std::max(0.0f, q31ToFloat(value));
    return floatToQ31(std::sqrt(v));
#endif
}

}  // namespace wibot
