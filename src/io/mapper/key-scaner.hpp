#pragma once

//
// Created by zhouj on 2024/3/25.
//
#include "model.hpp"
#include "system.hpp"

namespace wibot {

enum class KeyEvent : u8 {
    kNone,
    /**
     * Key pressed. Push down.
     */
    kPress,
    /**
     * Key hold. Long press.
     */
    kHold,
    /**
     * Key released. Push up.
     */
    kRelease,
    /**
     * Key released after hold.
     */
    kClick,
};

/**
 * @brief KeyScaner
 * key action event pattern:
 * click once: press, release, click
 * click twice: press, release, click, press, release, click. click count is 2.
 * hold: press, hold, release
 */
template <u8 CHANNELS>
class KeyScaner : public SyncPipeline<KeyEvent> {
   public:
    struct KeyScanerConfig {
        u16 holdThreshold;
        u16 clickIntervalThreshold;
    };

   public:
    KeyScaner(SyncPipeline<bool, u32>& upstream, KeyScanerConfig& config);
    KeyScaner(SyncPipeline<bool, u32>& upstream, u16 holdThreshold = 3000,
              u16 clickIntervalThreshold = 1000);

    void      update() override;
    void      reset() override;
    KeyEvent  getValue(u8 channel) const override;
    KeyEvent* getValues() const override;
    u8        getClickCount(u8 channel) const;

   protected:
    enum class KeyState : u8 {
        kNone,
        kPress,
        kHold,
        kRelease,
        kReleaseHold,
    };

   private:
    SyncPipeline<bool, u32>& _upstream;
    KeyScanerConfig          _config;

    u32      _pressTick[CHANNELS];
    u32      _clickTick[CHANNELS];
    KeyEvent _lastEvent[CHANNELS];
    KeyState _state[CHANNELS];
    u8       _clickCount[CHANNELS];
};

template <u8 CHANNELS>
KeyScaner<CHANNELS>::KeyScaner(SyncPipeline<bool, u32>& upstream, u16 holdThreshold,
                               u16 clickIntervalThreshold)
    : _upstream(upstream),
      _config{holdThreshold, clickIntervalThreshold},
      _pressTick{},
      _clickTick{},
      _lastEvent{KeyEvent::kNone},
      _state{KeyState::kNone},
      _clickCount{}{};

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

template <u8 CHANNELS>
void KeyScaner<CHANNELS>::reset() {
    for (u8 channel = 0; channel < CHANNELS; channel++) {
        _pressTick[channel]  = 0;
        _clickTick[channel]  = 0;
        _lastEvent[channel]  = KeyEvent::kNone;
        _state[channel]      = KeyState::kNone;
        _clickCount[channel] = 0;
    }
};

}  // namespace wibot
