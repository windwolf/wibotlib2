#pragma once

//
// Created by zhouj on 2023/9/8.
//
#include <stdint.h>
#include <stddef.h>
#include <type_traits>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using q15 = int16_t;
using q31 = int32_t;

template <typename T>
concept SupportArithmetic = std::is_arithmetic_v<T>;
template <typename T>
concept SupportInt = std::is_same_v<T, i8> || std::is_same_v<T, i16> || std::is_same_v<T, i32>;
template <typename T>
concept SupportUint = std::is_same_v<T, u8> || std::is_same_v<T, u16> || std::is_same_v<T, u32>;
template <typename T>
concept SupportFloat = std::is_same_v<T, f32> || std::is_same_v<T, f64>;
template <typename T>
concept SupportFloatOrQ = std::is_same_v<T, f32> || std::is_same_v<T, f64> ||
                          std::is_same_v<T, q15> || std::is_same_v<T, q31>;

#define TIMEOUT_NOWAIT  0x00000000
// There is no difference between any two known u32 values greater than 0xFFFFFFFF.
#define TIMEOUT_FOREVER 0xFFFFFFFF

#define ALIGN(n)      __attribute__((aligned(n)))
#define ALIGN32       __attribute__((aligned(32)))
#define PACKED        __attribute__((__packed__))
#define ALWAYS_INLINE __attribute__((always_inline)) inline

#ifndef offsetof
#define offsetof(type, member) \
    ((size_t)&reinterpret_cast<char const volatile &>((((type *)0)->member)))
#endif

#ifndef container_of
#define container_of(ptr, type, member)                                         \
    ((type *)((char *)static_cast<const decltype(((type *)0)->member) *>(ptr) - \
              offsetof(type, member)))
#endif

#define ASSERT(expr, msg, ...) \
    {                          \
        if (!(expr)) {         \
            while (true)       \
                ;              \
        }                      \
    }

namespace wibot {
// 移除模板函数以避免std依赖问题
// template <typename E>
// constexpr auto toUnderlying(E e) noexcept {
//     return static_cast<std::underlying_type_t<E>>(e);
// };

template <typename T>
constexpr auto castPointer(void *ptr) {
    return static_cast<T *>(ptr);
};

struct Result {
   public:
    enum class ResultStatus : u8 {
        kOk      = 0x00U,
        kError   = 0x01U,
        kBusy    = 0x02U,
        kTimeout = 0x03U,

        kInvalidParameter = 0x84U,
        kNoResource       = 0x85U,
        kNotSupport       = 0x86U,
    };

   public:
    Result();
    Result(const Result &other) = default;
    ~Result()                   = default;
    Result(ResultStatus status);
    Result(ResultStatus status, u32 errorCode);
    Result(u32 halStatus);

    bool isOk() const;
    bool isError() const;
    bool isBusy() const;
    bool isTimeout() const;

    u32 getErrorCode() const;

    bool operator==(const Result &other) const;
    bool operator!=(const Result &other) const;

   public:
   private:
    u32 value;  // 0xEEEEEESS. SS is HAL_StatusTypeDef, EEEEEE is error code.

   public:
    static const Result kOk;                // 静态常量成员
    static const Result kError;             // 静态常量成员
    static const Result kBusy;              // 静态常量成员
    static const Result kTimeout;           // 静态常量成员
    static const Result kInvalidParameter;  // 静态常量成员
    static const Result kNoResource;        // 静态常量成员
    static const Result kNotSupport;        // 静态常量成员
};

enum class Ternary : u8 {
    kTrue  = 0x01,
    kFalse = 0x00,
    kNone  = 0xFF,
};

enum class DataWidth : u8 {
    kNone   = 0x00,
    k8Bits  = 0x01,
    k16Bits = 0x02,
    k24Bits = 0x03,
    k32Bits = 0x04,
};

enum class Endian : bool {
    kLittle = 0,
    kBig    = 1,
};

class String {
   public:
    char *trim(char *str, char delimit) const;
};

class Codex {
   public:
    static u8 byteToBcd(u8 Value);

    static u8 bcdToByte(u8 Value);
};

// ------------------------
// LinkList

class LinkList {
   public:
    LinkList();

    LinkList *append(LinkList *node);

    LinkList *remove(LinkList *node);

    bool exist(LinkList *node);

   protected:
    LinkList *_next;
};

}  // namespace wibot
