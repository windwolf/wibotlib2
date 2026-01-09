#pragma once
#include "type.hpp"
#include "chip.hpp"
#include "peripheral.hpp"
#include "math.hpp"
#include "arch.hpp"

#ifndef WIBOT_CORDIC_THREAD_SAFE
#define WIBOT_CORDIC_THREAD_SAFE 1
#endif

namespace wibot {

#ifdef HAL_CORDIC_MODULE_ENABLED

class Cordic : private PeripheralBase {
   public:
    enum class Precision : u32 {
        kPrecisino1Cycles  = CORDIC_PRECISION_1CYCLE,
        kPrecision2Cycles  = CORDIC_PRECISION_2CYCLES,
        kPrecision3Cycles  = CORDIC_PRECISION_3CYCLES,
        kPrecision4Cycles  = CORDIC_PRECISION_4CYCLES,
        kPrecision5Cycles  = CORDIC_PRECISION_5CYCLES,
        kPrecision6Cycles  = CORDIC_PRECISION_6CYCLES,
        kPrecision7Cycles  = CORDIC_PRECISION_7CYCLES,
        kPrecision8Cycles  = CORDIC_PRECISION_8CYCLES,
        kPrecision9Cycles  = CORDIC_PRECISION_9CYCLES,
        kPrecision10Cycles = CORDIC_PRECISION_10CYCLES,
        kPrecision11Cycles = CORDIC_PRECISION_11CYCLES,
        kPrecision12Cycles = CORDIC_PRECISION_12CYCLES,
        kPrecision13Cycles = CORDIC_PRECISION_13CYCLES,
        kPrecision14Cycles = CORDIC_PRECISION_14CYCLES,
        kPrecision15Cycles = CORDIC_PRECISION_15CYCLES,
    };

   public:
    Cordic(CORDIC_HandleTypeDef& ins);
    ~Cordic();

    void setPrecision(Precision precision) {
        _precision = precision;
    }

    Vector2<q15> sincos(q15 angle, q15 modulus);
    Vector2<q31> sincos(q31 angle, q31 modulus);
    Vector2<q15> phaseModulus(q15 x, q15 y);
    Vector2<q31> phaseModulus(q31 x, q31 y);
    q15          atanh(q15 x);
    q31          atanh(q31 x);
    q15          log(q15 x);
    q31          log(q31 x);
    q15          sqrt(q15 x);
    q31          sqrt(q31 x);
    Vector2<q15> sincosh(q15 angle);
    Vector2<q31> sincosh(q31 angle);
    q15          atan(q15 x);
    q31          atan(q31 x);

   private:
    void lock();
    void unlock();

    CORDIC_HandleTypeDef* _ins;
    Precision             _precision = Precision::kPrecision12Cycles;
    volatile u32          _lock      = 0;
};

#endif
} // namespace wibot

