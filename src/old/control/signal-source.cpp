//
// Created by zhouj on 2023/8/28.
//

#include "signal-source.hpp"

namespace wibot {

SignalSource::SignalSource(SignalSourceMode mode, u8 initialValue)
    : _modeAndEventFlag(static_cast<u8>(mode)),
      _value(initialValue),
      _firstInput(true),
      _ignoreInitialValue(false) {
}

SignalSource::SignalSource(SignalSourceMode mode)
    : _modeAndEventFlag(static_cast<u8>(mode)),
      _value(0),
      _firstInput(true),
      _ignoreInitialValue(true) {
}

void SignalSource::update(bool value) {
    if (_firstInput) {
        _firstInput = false;
        if (_ignoreInitialValue) {
            _value = value ? 0b11111111 : 0b00000000;
        } else {
            _value = (_value << 1) | (value ? 0b00000001 : 0b00000000);
        }
    } else {
        _value = (_value << 1) | (value ? 0b00000001 : 0b00000000);
    }

    if (((_value ^ _modeAndEventFlag) & 0b00000011) == 0b00000000) {
        // event occurred
        _modeAndEventFlag |= 0b10000000;
    } else {
        // event not occurred
        _modeAndEventFlag &= 0b01111111;
    }
}

bool SignalSource::get() {
    return (_modeAndEventFlag & 0b10000000) == 0b10000000;
}

void SignalSource::clear() {
    _modeAndEventFlag &= 0b01111111;
}

void SignalSource::setMode(SignalSourceMode mode) {
    _modeAndEventFlag = (_modeAndEventFlag & 0b10000000) | toUnderlying(mode);
}

}  // namespace wibot
