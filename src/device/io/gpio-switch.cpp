//
// Created by zhouj on 2024/1/24.
//

#include "gpio-switch.hpp"
#include "hal/system.hpp"
namespace wibot {
GpioSwitch::GpioSwitch(Pin& pinIn, Pin& pinOut, GpioSwitch::Mode mode, u32 delay)
    : _pinIn(pinIn), _pinOut(pinOut), _mode(mode), _delay(delay) {
}

void GpioSwitch::update() {
    if (_mode == Mode::kSync) {
        updateSync();
    } else if (_mode == Mode::kHold) {
        updateHold();
    }
}

void GpioSwitch::updateSync() {
    bool in  = _pinIn.getValue();
    auto now = System::getTickMs();

    if (in != _lastInState) {
        _lastInState       = in;
        _lastInTriggerTick = now;
    }

    if (now - _lastInTriggerTick >= _delay) {
        _outState = _lastInState;
        _pinOut.setValue(_outState);
    }
}

void GpioSwitch::updateHold() {
    bool in  = _pinIn.getValue();
    auto now = System::getTickMs();
    if (!_lastInState || in) {
        _lastInState       = in;
        _lastInTriggerTick = now;
    }

    if (_lastInState && ((now - _lastInTriggerTick) > _delay)) {
        _outState = !_outState;
        _pinOut.setValue(_outState);
        _lastInState = false;
    }
}
}  // namespace wibot
