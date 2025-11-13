//
// Created by zhouj on 2024/3/25.
//

#include "key-scaner.hpp"
#include "system.hpp"

namespace wibot {

template <u8 CHANNELS>
KeyScaner<CHANNELS>::KeyScaner(SyncPipeline<bool, u32>& upstream, u16 holdThreshold,
                               u16 clickIntervalThreshold)
    : _upstream(upstream),
      _config{holdThreshold, clickIntervalThreshold},
      _pressTick({0}),
      _clickTick({0}),
      _lastEvent({KeyEvent::kNone}),
      _state({KeyState::kNone}),
      _clickCount({0}){};

template <u8 CHANNELS>
KeyScaner<CHANNELS>::KeyScaner(SyncPipeline<bool, u32>& upstream, KeyScanerConfig& config)
    : KeyScaner(upstream, config.holdThreshold, config.clickIntervalThreshold){};

template <u8 CHANNELS>
u8 KeyScaner<CHANNELS>::getClickCount(u8 channel) const {
    return _clickCount[channel];
};

template <u8 CHANNELS>
KeyEvent KeyScaner<CHANNELS>::getValue(u8 channel) const {
    auto event = _lastEvent[channel];
    return event;
};

template <u8 CHANNELS>
KeyEvent* KeyScaner<CHANNELS>::getValues() const {
    return const_cast<KeyEvent*>(_lastEvent);
};

template <u8 CHANNELS>
void KeyScaner<CHANNELS>::update() {
    _upstream.update();
    auto _pinStatus = _upstream.getValues();
    auto now        = System::getTickMs();

    // Update digital input and get current pin status

    for (u8 channel = 0; channel < CHANNELS; channel++) {
        // Get the current pin status for this channel
        bool currentPinStatus = (_pinStatus & (1U << channel));

        switch (_state[channel]) {
            case KeyState::kNone:
                if (currentPinStatus) {
                    _pressTick[channel] = now;
                    _state[channel]     = KeyState::kPress;
                    _lastEvent[channel] = KeyEvent::kPress;
                }
                break;
            case KeyState::kPress:
                if (currentPinStatus) {
                    if ((now - _pressTick[channel]) > _config.holdThreshold) {
                        _state[channel]     = KeyState::kHold;
                        _lastEvent[channel] = KeyEvent::kHold;
                    }
                } else {
                    _state[channel]     = KeyState::kRelease;
                    _lastEvent[channel] = KeyEvent::kRelease;
                }
                break;
            case KeyState::kHold:
                if (!currentPinStatus) {
                    _state[channel]     = KeyState::kReleaseHold;
                    _lastEvent[channel] = KeyEvent::kRelease;
                }
                break;
            case KeyState::kRelease:
                if ((now - _clickTick[channel]) < _config.clickIntervalThreshold) {
                    _clickCount[channel]++;
                } else {
                    _clickCount[channel] = 1;
                }
                _clickTick[channel] = now;
                _state[channel]     = KeyState::kNone;
                _lastEvent[channel] = KeyEvent::kClick;
                break;
            case KeyState::kReleaseHold:
                _state[channel]      = KeyState::kNone;
                _clickCount[channel] = 0;
                break;
        }
    }
};

}  // namespace wibot
