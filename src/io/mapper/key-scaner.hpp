#pragma once

//
// Created by zhouj on 2024/3/25.
//
#include "model.hpp"
#include "system.hpp"
#include "../source/digital-source.hpp"

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

struct KeyScanerConfig {
    u16 holdThreshold;
    u16 clickIntervalThreshold;
};

/**
 * @brief 多通道按键扫描器
 * 
 * 处理多个按键通道的事件检测。
 * 保留多通道设计,因为按键通常是多个一起使用的硬件场景。
 * 
 * 继承 MultiChannelPipeline<KeyEvent, CHANNELS> 接口。
 * 可通过 ChannelAdapter 将特定通道适配为单通道 SyncPipeline。
 * 
 * key action event pattern:
 * click once: press, release, click
 * click twice: press, release, click, press, release, click. click count is 2.
 * hold: press, hold, release
 * 
 * @tparam CHANNELS 按键通道数量
 */
template <u8 CHANNELS>
class KeyScaner : public MultiChannelPipeline<KeyEvent, CHANNELS> {
   public:
   public:
    KeyScaner(DigitalSource<CHANNELS>& upstream, KeyScanerConfig& config)
        : _upstream(upstream),
          _config(config),
          _pressTick{},
          _clickTick{},
          _lastEvent{KeyEvent::kNone},
          _state{KeyState::kNone},
          _clickCount{} {};

    void update() {
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
    }

    void reset() {
        for (u8 channel = 0; channel < CHANNELS; channel++) {
            _pressTick[channel]  = 0;
            _clickTick[channel]  = 0;
            _lastEvent[channel]  = KeyEvent::kNone;
            _state[channel]      = KeyState::kNone;
            _clickCount[channel] = 0;
        }
    }

    /**
     * @brief 获取指定通道的按键事件
     */
    KeyEvent getValue(u8 channel) const {
        if (channel >= CHANNELS) {
            return KeyEvent::kNone;
        }
        return _lastEvent[channel];
    }

    /**
     * @brief 获取指定通道的点击次数
     */
    u8 getClickCount(u8 channel) const {
        if (channel >= CHANNELS) {
            return 0;
        }
        return _clickCount[channel];
    }

   protected:
    enum class KeyState : u8 {
        kNone,
        kPress,
        kHold,
        kRelease,
        kReleaseHold,
    };

   private:
    DigitalSource<CHANNELS>& _upstream;
    KeyScanerConfig&         _config;

    u32      _pressTick[CHANNELS];
    u32      _clickTick[CHANNELS];
    KeyEvent _lastEvent[CHANNELS];
    KeyState _state[CHANNELS];
    u8       _clickCount[CHANNELS];
};

}  // namespace wibot
