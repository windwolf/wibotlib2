#pragma once

#include "type.hpp"

namespace wibot {

constexpr f32 kPI      = 3.14159265358979323846f;  // π
constexpr f32 k2PI     = 6.28318530717958647692f;  // 2π
constexpr f32 kPI_2    = 1.57079632679489661923f;  // π/2
constexpr f32 kPI_3    = 1.04719755119659774615f;  // π/3
constexpr f32 k2PI_3   = 2.09439510239319549231f;  // 2π/3
constexpr f32 k4PI_3   = 4.18879020478639098462f;  // 4π/3
constexpr f32 k5PI_3   = 5.23598775598298873078f;  // 5π/3
constexpr f32 kSQRT3   = 1.73205080756887729352f;  // √(3)
constexpr f32 kSQRT3_2 = 0.86602540378443864676f;  // √(3)/2
constexpr f32 k1_SQRT3 = 0.57735026918962576450f;  // 1/√(3)
constexpr f32 k2_SQRT3 = 1.15470053837925152900f;  // 2/√(3)
constexpr f32 k1_3     = 0.33333333333333333333f;  // 1/3
constexpr f32 kSQRT2_3 = 0.81649658092772603273f;  // √(2/3)
constexpr f32 kSQRT2_9 = 0.47140452079103168293f;  // √(2)/3

constexpr f32 kQ15Scale = 32768.0f;
constexpr f32 kQ31Scale = 2147483648.0f;

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

}  // namespace wibot
