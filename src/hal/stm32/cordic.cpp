#include "cordic.hpp"

#ifdef HAL_CORDIC_MODULE_ENABLED

namespace wibot::hal {

Cordic::Cordic(CORDIC_HandleTypeDef& ins) : _ins(&ins) {
    PeripheralManager::getInstance().registerPeripheral(this, _ins);
}

Cordic::~Cordic() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
}

void Cordic::lock() {
#if WIBOT_CORDIC_THREAD_SAFE
    while (!arch::syncCompareAndSwap(&_lock, 0, 1)) {
    }
#endif
}

void Cordic::unlock() {
#if WIBOT_CORDIC_THREAD_SAFE
    _lock = 0;
#endif
}

Vector2<q15> Cordic::sincos(q15 angle, q15 modulus) {
    lock();
    int32_t              in[2]  = {static_cast<int32_t>(angle), static_cast<int32_t>(modulus)};
    int32_t              out[2] = {0, 0};
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_SINE,        CORDIC_SCALE_0,   CORDIC_INSIZE_16BITS,
                                CORDIC_OUTSIZE_16BITS,       CORDIC_NBWRITE_2, CORDIC_NBREAD_2,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, in, out, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q15>(out[0]), static_cast<q15>(out[1])};
}

Vector2<q31> Cordic::sincos(q31 angle, q31 modulus) {
    lock();
    int32_t              in[2]  = {static_cast<int32_t>(angle), static_cast<int32_t>(modulus)};
    int32_t              out[2] = {0, 0};
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_SINE,        CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_2, CORDIC_NBREAD_2,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, in, out, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q31>(out[0]), static_cast<q31>(out[1])};
}

Vector2<q15> Cordic::phaseModulus(q15 x, q15 y) {
    lock();
    int32_t in[2] = {static_cast<int32_t>(x), static_cast<int32_t>(y)};
    int32_t phase = 0;
    int32_t mod   = 0;

    CORDIC_ConfigTypeDef phaseCfg = {
        CORDIC_FUNCTION_PHASE, CORDIC_SCALE_0,  CORDIC_INSIZE_16BITS,        CORDIC_OUTSIZE_16BITS,
        CORDIC_NBWRITE_2,      CORDIC_NBREAD_1, static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &phaseCfg);
    HAL_CORDIC_Calculate(_ins, in, &phase, 1, HAL_MAX_DELAY);

    CORDIC_ConfigTypeDef modCfg = {
        CORDIC_FUNCTION_MODULUS,     CORDIC_SCALE_0,   CORDIC_INSIZE_16BITS,
        CORDIC_OUTSIZE_16BITS,       CORDIC_NBWRITE_2, CORDIC_NBREAD_1,
        static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &modCfg);
    HAL_CORDIC_Calculate(_ins, in, &mod, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q15>(phase), static_cast<q15>(mod)};
}

Vector2<q31> Cordic::phaseModulus(q31 x, q31 y) {
    lock();
    int32_t in[2] = {static_cast<int32_t>(x), static_cast<int32_t>(y)};
    int32_t phase = 0;
    int32_t mod   = 0;

    CORDIC_ConfigTypeDef phaseCfg = {
        CORDIC_FUNCTION_PHASE, CORDIC_SCALE_0,  CORDIC_INSIZE_32BITS,        CORDIC_OUTSIZE_32BITS,
        CORDIC_NBWRITE_2,      CORDIC_NBREAD_1, static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &phaseCfg);
    HAL_CORDIC_Calculate(_ins, in, &phase, 1, HAL_MAX_DELAY);

    CORDIC_ConfigTypeDef modCfg = {
        CORDIC_FUNCTION_MODULUS,     CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
        CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_2, CORDIC_NBREAD_1,
        static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &modCfg);
    HAL_CORDIC_Calculate(_ins, in, &mod, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q31>(phase), static_cast<q31>(mod)};
}

q15 Cordic::atanh(q15 x) {
    lock();
    int32_t              in  = static_cast<int32_t>(x) << 16;
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_HARCTANGENT, CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q15>(out >> 16);
}

q31 Cordic::atanh(q31 x) {
    lock();
    int32_t              out = 0;
    int32_t              in  = static_cast<int32_t>(x);
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_HARCTANGENT, CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q31>(out);
}

q15 Cordic::log(q15 x) {
    if (x <= 0) {
        return 0;
    }
    lock();
    int32_t              in  = static_cast<int32_t>(x) << 16;
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_NATURALLOG,  CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q15>(out >> 16);
}

q31 Cordic::log(q31 x) {
    if (x <= 0) {
        return 0;
    }
    lock();
    int32_t              in  = static_cast<int32_t>(x);
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_NATURALLOG,  CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q31>(out);
}

q15 Cordic::sqrt(q15 x) {
    if (x <= 0) {
        return 0;
    }
    lock();
    int32_t              in  = static_cast<int32_t>(x) << 16;
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_SQUAREROOT,  CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q15>(out >> 16);
}

q31 Cordic::sqrt(q31 x) {
    if (x <= 0) {
        return 0;
    }
    lock();
    int32_t              in  = static_cast<int32_t>(x);
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_SQUAREROOT,  CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q31>(out);
}

Vector2<q15> Cordic::sincosh(q15 angle) {
    lock();
    int32_t              in     = static_cast<int32_t>(angle);
    int32_t              out[2] = {0, 0};
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_HSINE,       CORDIC_SCALE_0,   CORDIC_INSIZE_16BITS,
                                CORDIC_OUTSIZE_16BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_2,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, out, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q15>(out[0]), static_cast<q15>(out[1])};
}

Vector2<q31> Cordic::sincosh(q31 angle) {
    lock();
    int32_t              in     = static_cast<int32_t>(angle);
    int32_t              out[2] = {0, 0};
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_HSINE,       CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_2,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, out, 1, HAL_MAX_DELAY);
    unlock();
    return {static_cast<q31>(out[0]), static_cast<q31>(out[1])};
}

q15 Cordic::atan(q15 x) {
    lock();
    int32_t              in  = static_cast<int32_t>(x);
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_ARCTANGENT,  CORDIC_SCALE_0,   CORDIC_INSIZE_16BITS,
                                CORDIC_OUTSIZE_16BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q15>(out);
}

q31 Cordic::atan(q31 x) {
    lock();
    int32_t              in  = static_cast<int32_t>(x);
    int32_t              out = 0;
    CORDIC_ConfigTypeDef cfg = {CORDIC_FUNCTION_ARCTANGENT,  CORDIC_SCALE_0,   CORDIC_INSIZE_32BITS,
                                CORDIC_OUTSIZE_32BITS,       CORDIC_NBWRITE_1, CORDIC_NBREAD_1,
                                static_cast<u32>(_precision)};
    HAL_CORDIC_Configure(_ins, &cfg);
    HAL_CORDIC_Calculate(_ins, &in, &out, 1, HAL_MAX_DELAY);
    unlock();
    return static_cast<q31>(out);
}

}  // namespace wibot

#endif
