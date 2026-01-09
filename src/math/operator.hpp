#pragma once

#include <cmath>
#include <type_traits>
#include "math.hpp"

#include "hal/stm32/cordic.hpp"

namespace wibot::math {

class Math {
   public:
    Math();
#ifdef HAL_CORDIC_MODULE_ENABLED

    Math(hal::Cordic& hcordic);
#endif
    // 基础运算
    template <SupportArithmetic T>
    T add(const T a, const T b) {
        return a + b;
    };
    template <SupportArithmetic T>
    T sub(const T a, const T b) {
        return a - b;
    };
    template <SupportArithmetic T>
    T mul(const T a, const T b) {
        return a * b;
    };
    template <SupportArithmetic T>
    T sign(const T a) {
        return (a > 0) ? static_cast<T>(1) : ((a < 0) ? static_cast<T>(-1) : static_cast<T>(0));
    };
    template <SupportArithmetic T>
    Vector2<T> sign(const Vector2<T> a) {
        return {sign(a.v1), sign(a.v2)};
    };

    template <SupportFloat T>
    T mod(const T x, const T y) {
        f32 r = std::fmod(x, y);
        if (r < 0.0f && y > 0.0f) {
            r += y;
        }
        return r;
    };

    template <SupportFloat T>
    T circleNormalize(const T theta) {
        theta = std::fmod(theta, k2PI);
        if (theta > kPI) {
            theta -= k2PI;
        } else if (theta < -kPI) {
            theta += k2PI;
        }
        return theta;
    };

    template <SupportFloat T>
    T floor(const T x) {
        return std::floor(x);
    };

    template <SupportUint T>
    T log2(const T val) {
        if (val == 0U) {
            return 0;
        }
#if defined(__GNUC__)
        return 31U - static_cast<u32>(__builtin_clz(val));
#else
        u32 l = 0;
        while ((val >> l) > 1U) {
            l++;
        }
        return l;
#endif
    };

    // 三角函数
    template <SupportFloatOrQ T>
    Vector2<T> sincos(const T angle, const T modulus);

    template <SupportFloatOrQ T>
    Vector2<T> phaseModulus(const T x, const T y);

    template <SupportFloatOrQ T>
    T atan2(T x);

    // 双曲线函数
    template <SupportFloatOrQ T>
    Vector2<T> sincosh(const T angle);

    template <SupportFloatOrQ T>
    T atanh2(const T x);

    // 对数和平方根
    template <SupportFloatOrQ T>
    T log(const T value);
    template <SupportFloatOrQ T>
    T sqrt(const T value);

   private:
#ifdef HAL_CORDIC_MODULE_ENABLED
    hal::Cordic* _cordic;
#endif
};

};  // namespace wibot
