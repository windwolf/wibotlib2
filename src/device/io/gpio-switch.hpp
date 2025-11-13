#pragma once

//
// Created by zhouj on 2024/1/24.
//

#include "gpio.hpp"

namespace wibot {
class GpioSwitch {
   public:
    enum class Mode {
        kSync,
        kHold,
    };

   public:
    GpioSwitch(Pin& pinIn, Pin& pinOut, Mode mode = Mode::kSync, u32 delay = 0);

    void update();

   private:
    Pin& _pinIn;
    Pin& _pinOut;
    Mode _mode;
    u32  _delay;
    bool _lastInState;
    u32  _lastInTriggerTick;
    bool _outState;

    void updateSync();
    void updateHold();
};

}  // namespace wibot
