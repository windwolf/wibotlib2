#pragma once

//
// Created by zhouj on 2024/1/24.
//

#include "hal/stm32/gpio.hpp"

namespace wibot::device {
class GpioSwitch {
   public:
    enum class Mode {
        kSync,
        kHold,
    };

   public:
    GpioSwitch(hal::Pin& pinIn, hal::Pin& pinOut, Mode mode = Mode::kSync, u32 delay = 0);

    void update();

   private:
    hal::Pin& _pinIn;
    hal::Pin& _pinOut;
    Mode      _mode;
    u32       _delay;
    bool      _lastInState;
    u32       _lastInTriggerTick;
    bool      _outState;

    void updateSync();
    void updateHold();
};

}  // namespace wibot::device
