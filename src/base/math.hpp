#pragma once

//
// Created by zhouj on 2023/9/8.
//

#include <cmath>
#include "type.hpp"
// #include "arm_math.h"

namespace wibot {

constexpr f32 kPI      = 3.14159265358979323846f;
constexpr f32 k2PI     = 6.28318530717958647692f;
constexpr f32 kPI_2    = 1.57079632679489661923f;
constexpr f32 kPI_3    = 1.04719755119659774615f;
constexpr f32 k2PI_3   = 2.09439510239319549231f;
constexpr f32 k4PI_3   = 4.18879020478639098462f;
constexpr f32 k5PI_3   = 5.23598775598298873078f;
constexpr f32 kSQRT3   = 1.73205080756887729352f;
constexpr f32 kSQRT3_2 = 0.86602540378443864676f;
constexpr f32 k1_SQRT3 = 0.57735026918962576450f;
constexpr f32 k2_SQRT3 = 1.15470053837925152900f;
constexpr f32 k1_3     = 0.33333333333333333333f;

// -------------------------------
// Vector2, Vector3, Vector4

template <typename T>
struct Vector2 {
    T v1;
    T v2;

    Vector2() : v1(0), v2(0) {
    }
    Vector2(T v1, T v2) : v1(v1), v2(v2) {
    }
};
template <typename T>
static Vector2<T> operator+(const Vector2<T>& one, const Vector2<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 + other.v1;
    result.v2 = one.v2 + other.v2;
    return result;
};
template <typename T>
Vector2<T> operator+(const Vector2<T>& one, const T other) {
    Vector2<T> result;
    result.v1 = one.v1 + other;
    result.v2 = one.v2 + other;
    return result;
};
template <typename T>
void operator+=(const Vector2<T>& one, const Vector2<T>& other) {
    one.v1 += other.v1;
    one.v2 += other.v2;
};
template <typename T>
void operator+=(const Vector2<T>& one, const T other) {
    one.v1 += other;
    one.v2 += other;
};
template <typename T>
Vector2<T> operator-(const Vector2<T>& one, const Vector2<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 - other.v1;
    result.v2 = one.v2 - other.v2;
    return result;
};
template <typename T>
Vector2<T> operator-(const Vector2<T>& one, const T& other) {
    Vector2<T> result;
    result.v1 = one.v1 - other;
    result.v2 = one.v2 - other;
    return result;
};
template <typename T>
void operator-=(const Vector2<T>& one, const Vector2<T>& other) {
    one.v1 -= other.v1;
    one.v2 -= other.v2;
};
template <typename T>
void operator-=(const Vector2<T>& one, const T other) {
    one.v1 -= other;
    one.v2 -= other;
};
template <typename T>
Vector2<T> operator*(const Vector2<T>& one, const Vector2<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 * other.v1;
    result.v2 = one.v2 * other.v2;
    return result;
};
template <typename T>
Vector2<T> operator*(const Vector2<T>& one, const T other) {
    Vector2<T> result;
    result.v1 = one.v1 * other;
    result.v2 = one.v2 * other;
    return result;
};
template <typename T>
void operator*=(const Vector2<T>& one, const Vector2<T>& other) {
    one.v1 *= other.v1;
    one.v2 *= other.v2;
};
template <typename T>
void operator*=(const Vector2<T>& one, const T other) {
    one.v1 *= other;
    one.v2 *= other;
};
template <typename T>
Vector2<T> operator/(const Vector2<T>& one, const Vector2<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 / other.v1;
    result.v2 = one.v2 / other.v2;
    return result;
};
template <typename T>
Vector2<T> operator/(const Vector2<T>& one, const T other) {
    Vector2<T> result;
    result.v1 = one.v1 / other;
    result.v2 = one.v2 / other;
    return result;
};
template <typename T>
void operator/=(const Vector2<T>& one, const Vector2<T>& other) {
    one.v1 /= other.v1;
    one.v2 /= other.v2;
};
template <typename T>
void operator/=(const Vector2<T>& one, const T other) {
    one.v1 /= other;
    one.v2 /= other;
};

using Vector2f = Vector2<f32>;
using Vector2i = Vector2<u32>;

template <typename T>
struct Vector3 {
    T v1;
    T v2;
    T v3;

    Vector3() : v1(0), v2(0), v3(0) {};
    Vector3(T v1, T v2, T v3) : v1(v1), v2(v2), v3(v3) {};
};
template <typename T>
Vector3<T> operator+(const Vector3<T>& one, const Vector3<T>& other) {
    Vector3<T> result;
    result.v1 = one.v1 + other.v1;
    result.v2 = one.v2 + other.v2;
    result.v3 = one.v3 + other.v3;
    return result;
};
template <typename T>
Vector3<T> operator+(const Vector3<T>& one, const T other) {
    Vector3<T> result;
    result.v1 = one.v1 + other;
    result.v2 = one.v2 + other;
    result.v3 = one.v3 + other;
    return result;
};
template <typename T>
void operator+=(const Vector3<T>& one, const Vector3<T>& other) {
    one.v1 += other.v1;
    one.v2 += other.v2;
    one.v3 += other.v3;
};
template <typename T>
void operator+=(const Vector3<T>& one, const T other) {
    one.v1 += other;
    one.v2 += other;
    one.v3 += other;
};
template <typename T>
Vector3<T> operator-(const Vector3<T>& one, const Vector3<T>& other) {
    Vector3<T> result;
    result.v1 = one.v1 - other.v1;
    result.v2 = one.v2 - other.v2;
    result.v3 = one.v3 - other.v3;
    return result;
};
template <typename T>
Vector3<T> operator-(const Vector3<T>& one, const T& other) {
    Vector3<T> result;
    result.v1 = one.v1 - other;
    result.v2 = one.v2 - other;
    result.v3 = one.v3 - other;
    return result;
};
template <typename T>
void operator-=(const Vector3<T>& one, const Vector3<T>& other) {
    one.v1 -= other.v1;
    one.v2 -= other.v2;
    one.v3 -= other.v3;
};
template <typename T>
void operator-=(const Vector3<T>& one, const T other) {
    one.v1 -= other;
    one.v2 -= other;
    one.v3 -= other;
};
template <typename T>
Vector3<T> operator*(const Vector3<T>& one, const Vector3<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 * other.v1;
    result.v2 = one.v2 * other.v2;
    result.v3 = one.v3 * other.v3;
    return result;
};
template <typename T>
Vector3<T> operator*(const Vector3<T>& one, const T other) {
    Vector3<T> result;
    result.v1 = one.v1 * other;
    result.v2 = one.v2 * other;
    result.v3 = one.v3 * other;
    return result;
};
template <typename T>
void operator*=(const Vector3<T>& one, const Vector3<T>& other) {
    one.v1 *= other.v1;
    one.v2 *= other.v2;
    one.v3 *= other.v3;
};
template <typename T>
void operator*=(const Vector3<T>& one, const T other) {
    one.v1 *= other;
    one.v2 *= other;
    one.v3 *= other;
};
template <typename T>

Vector3<T> operator/(const Vector3<T>& one, const Vector3<T>& other) {
    Vector3<T> result;
    result.v1 = one.v1 / other.v1;
    result.v2 = one.v2 / other.v2;
    result.v3 = one.v3 / other.v3;
    return result;
};
template <typename T>
Vector3<T> operator/(const Vector3<T>& one, const T other) {
    Vector3<T> result;
    result.v1 = one.v1 / other;
    result.v2 = one.v2 / other;
    result.v3 = one.v3 / other;
    return result;
};
template <typename T>
void operator/=(const Vector3<T>& one, const Vector3<T>& other) {
    one.v1 /= other.v1;
    one.v2 /= other.v2;
    one.v3 /= other.v3;
};
template <typename T>
void operator/=(const Vector3<T>& one, const T other) {
    one.v1 /= other;
    one.v2 /= other;
    one.v3 /= other;
};

using Vector3f = Vector3<f32>;
using Vector3b = Vector3<u8>;
using Vector3i = Vector3<u32>;

template <typename T>
struct Vector4 {
    T v1;
    T v2;
    T v3;
    T v4;

    Vector4() : v1(0), v2(0), v3(0), v4(0) {};
    Vector4(T v1, T v2, T v3, T v4) : v1(v1), v2(v2), v3(v3), v4(v4) {};
};
template <typename T>
Vector4<T> operator+(const Vector4<T>& one, const Vector4<T>& other) {
    Vector4<T> result;
    result.v1 = one.v1 + other.v1;
    result.v2 = one.v2 + other.v2;
    result.v3 = one.v3 + other.v3;
    result.v4 = one.v4 + other.v4;
    return result;
};
template <typename T>
Vector4<T> operator+(const Vector4<T>& one, const T other) {
    Vector4<T> result;
    result.v1 = one.v1 + other;
    result.v2 = one.v2 + other;
    result.v3 = one.v3 + other;
    result.v4 = one.v4 + other;
    return result;
};
template <typename T>
void operator+=(const Vector4<T>& one, const Vector4<T>& other) {
    one.v1 += other.v1;
    one.v2 += other.v2;
    one.v3 += other.v3;
    one.v4 += other.v4;
};
template <typename T>
void operator+=(const Vector4<T>& one, const T other) {
    one.v1 += other;
    one.v2 += other;
    one.v3 += other;
    one.v4 += other;
};
template <typename T>
Vector4<T> operator-(const Vector4<T>& one, const Vector4<T>& other) {
    Vector4<T> result;
    result.v1 = one.v1 - other.v1;
    result.v2 = one.v2 - other.v2;
    result.v3 = one.v3 - other.v3;
    result.v4 = one.v4 - other.v4;
    return result;
};
template <typename T>
Vector4<T> operator-(const Vector4<T>& one, const T& other) {
    Vector4<T> result;
    result.v1 = one.v1 - other;
    result.v2 = one.v2 - other;
    result.v3 = one.v3 - other;
    result.v4 = one.v4 - other;
    return result;
};
template <typename T>
void operator-=(const Vector4<T>& one, const Vector4<T>& other) {
    one.v1 -= other.v1;
    one.v2 -= other.v2;
    one.v3 -= other.v3;
    one.v4 -= other.v4;
};
template <typename T>
void operator-=(const Vector4<T>& one, const T other) {
    one.v1 -= other;
    one.v2 -= other;
    one.v3 -= other;
    one.v4 -= other;
};
template <typename T>
Vector4<T> operator*(const Vector4<T>& one, const Vector4<T>& other) {
    Vector2<T> result;
    result.v1 = one.v1 * other.v1;
    result.v2 = one.v2 * other.v2;
    result.v3 = one.v3 * other.v3;
    result.v4 = one.v4 * other.v4;
    return result;
};
template <typename T>
Vector4<T> operator*(const Vector4<T>& one, const T other) {
    Vector4<T> result;
    result.v1 = one.v1 * other;
    result.v2 = one.v2 * other;
    result.v3 = one.v3 * other;
    result.v4 = one.v4 * other;
    return result;
};
template <typename T>
void operator*=(const Vector4<T>& one, const Vector4<T>& other) {
    one.v1 *= other.v1;
    one.v2 *= other.v2;
    one.v3 *= other.v3;
    one.v4 *= other.v4;
};
template <typename T>
void operator*=(const Vector4<T>& one, const T other) {
    one.v1 *= other;
    one.v2 *= other;
    one.v3 *= other;
    one.v4 *= other;
};
template <typename T>
Vector4<T> operator/(const Vector4<T>& one, const Vector4<T>& other) {
    Vector4<T> result;
    result.v1 = one.v1 / other.v1;
    result.v2 = one.v2 / other.v2;
    result.v3 = one.v3 / other.v3;
    result.v4 = one.v4 / other.v4;
    return result;
};
template <typename T>
Vector4<T> operator/(const Vector4<T>& one, const T other) {
    Vector4<T> result;
    result.v1 = one.v1 / other;
    result.v2 = one.v2 / other;
    result.v3 = one.v3 / other;
    result.v4 = one.v4 / other;
    return result;
};
template <typename T>
void operator/=(const Vector4<T>& one, const Vector4<T>& other) {
    one.v1 /= other.v1;
    one.v2 /= other.v2;
    one.v3 /= other.v3;
    one.v4 /= other.v4;
};
template <typename T>
void operator/=(const Vector4<T>& one, const T other) {
    one.v1 /= other;
    one.v2 /= other;
    one.v3 /= other;
    one.v4 /= other;
};
using Vector4f = Vector4<f32>;
using Vector4b = Vector4<u8>;
using Vector4i = Vector4<u32>;

#define MATH_MAT_ROW(mat, row)          ((mat).pData + row * (mat).numCols)
#define MATH_MAT_COLUMN(mat, col)       ((mat).pData + col)
#define MATH_MAT_ELEMENT(mat, row, col) ((mat).pData + row * (mat).numCols + col)

#define MATH_MAT_F32_DECLARE(mat, row, col)            \
    arm_matrix_instance_f32 mat;                       \
    float32_t               mat##_data[(row) * (col)]; \
    arm_mat_init_f32(&mat, (row), (col), mat##_data)

#define MATH_MAT_F32_SET(mat, val) \
    memset((mat)->pData, val, (mat)->numRows*(mat)->numCols * sizeof(float32_t))

#define MATH_MAT_F32_COPY(dst, src) \
    memcpy((dst)->pData, (src)->pData, (dst)->numRows*(dst)->numCols * sizeof(float32_t))

// template <u16 rowNum, u16 colNum>
// struct Matrix_f32 {
//     Matrix_f32() {
//         arm_mat_init_f32(&mat, (rowNum), (colNum), data);
//     };
//     Matrix_f32(const Matrix_f32& obj) {
//         arm_mat_init_f32(&mat, (rowNum), (colNum), data);
//         memcpy(data, obj.data, sizeof(data));
//     }
//     void setValue(float32_t val) {
//         memset(data, val, sizeof(data));
//     }

//     float32_t               data[rowNum * colNum];
//     arm_matrix_instance_f32 mat;
// };

struct Math {
    // static f32 atan2(f32 y, f32 x) {
    //     f32 result;
    //     arm_atan2_f32(y, x, &result);
    //     return result;
    // }
    // static void sincos(f32 theta, f32* sin, f32* cos) {
    //     arm_sin_cos_f32(theta, sin, cos);
    // }

    // static f32 sin(f32 theta) {
    //     return arm_sin_f32(theta);
    // }

    // static f32 cos(f32 theta) {
    //     return arm_cos_f32(theta);
    // }

    // static f32 sqrt(f32 x) {
    //     f32 result;
    //     arm_sqrt_f32(x, &result);
    //     return result;
    // }

    static f32 mod(f32 x, f32 y) {
        f32 result = fmod(x, y);
        return result >= 0 ? result : (result + y);
    }

    static f32 sign(f32 x) {
        return x >= 0 ? 1 : -1;
    }

    static Vector2f sign(Vector2f x) {
        Vector2f result;
        if (x.v1 > 0)
            result.v1 = 1.0f;
        else if (x.v1 < 0)

            result.v1 = -1.0f;
        else
            result.v1 = 0.0f;

        if (x.v2 > 0)
            result.v2 = 1.0f;
        else if (x.v2 < 0)
            result.v2 = -1.0f;
        else
            result.v2 = 0.0f;
        return result;
    }

    static f32 circleNormalize(f32 theta) {
        f32 result = fmod(theta, k2PI);
        return result >= 0 ? result : (result + k2PI);
    }

    static f32 floor(f32 x) {
        return ::floorf(x);
    }

    static u32 fastLog2(u32 val);
};

}  // namespace wibot
