#pragma once

//
// Created by zhouj on 2023/8/28.
//

#include "type.hpp"

namespace wibot {

enum class SignalSourceMode : u8 {
    kRiseEdge  = 0b00000001,
    kFallEdge  = 0b00000010,
    kHighLevel = 0b00000011,
    kLowLevel  = 0b00000000,
};

struct TriggerSourceConfig {
    SignalSourceMode mode;
};

/**
 * lastValue    value   mode        eventFlag
 * 0            1       RiseEdge    1
 * 1            0       FallEdge    1
 * x            1       HighLevel   1
 * x            0       LowLevel    1
 * otherwise                        0
 */
class SignalSource {
   public:
    /**
     * @brief Construct a new Signal Source object
     * @note No initial value, when first value input, all the history value will be input value.
     * @param mode
     */
    explicit SignalSource(SignalSourceMode mode);

    /**
     * @brief Construct a new Signal Source object
     * @param mode
     * @param initialValue
     */
    SignalSource(SignalSourceMode mode, u8 initialValue);

    void setMode(SignalSourceMode mode);
    void update(bool value);
    bool get();
    void clear();

   private:
    /**
     * bit0-1: mode
     * bit7: event flag
     */
    u8 _modeAndEventFlag;
    /**
     * bit0: current value
     * bit1-7: last value
     */
    u8 _value;

    bool _firstInput;
    bool _ignoreInitialValue;
};
}  // namespace wibot
