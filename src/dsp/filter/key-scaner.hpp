#pragma once

#include "type.hpp"

namespace wibot::dsp {

/**
 * @brief 多通道按键扫描核心
 * 
 * 处理多个按键通道的事件检测，逻辑与 io 层保持一致：按下、长按、释放、单击、双击。

 * 完整的按键事件模式：
 * click once: press, release, click
 * click twice: press, release, click, press, release, click2.
 * hold: press, hold, release
 * click then hold: press, release, click, press, hold, release
 * @tparam CHANNELS 按键通道数量
 */
template <u8 CHANNELS>
class KeyScaner {
   public:
    enum class KeyEvent : u8 {
        kNone,
        kPress,    // 按下
        kHold,     // 长按
        kRelease,  // 释放
        kClick,    // 单击（释放后）
        kClick2,   // 连击2次
    };

    enum class KeyState : u8 {
        kNone,
        kPress,
        kHold,
        kRelease,
        kReleaseHold,
    };

    struct Config {
        u16 holdThreshold{500};           // 长按阈值 (ms)
        u16 clickIntervalThreshold{300};  // 连击间隔阈值 (ms)
    };

    struct State {
        u32      pressTick[CHANNELS]{};
        u32      clickTick[CHANNELS]{};
        KeyEvent lastEvent[CHANNELS]{KeyEvent::kNone};
        KeyState state[CHANNELS]{KeyState::kNone};
    };

   public:
    explicit KeyScaner(Config& config) : _config(config) {
    }

    void reset() {
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            _state.pressTick[ch] = 0;
            _state.clickTick[ch] = 0;
            _state.lastEvent[ch] = KeyEvent::kNone;
            _state.state[ch]     = KeyState::kNone;
        }
    }

    /**
     * @brief 处理按键输入（多通道位掩码）
     *
     * @param pinStatusMask 位掩码，bit=1 表示对应通道按下
     * @param currentTick   当前时刻 (ms)
     */
    void scan(u32 pinStatusMask, u32 currentTick) {
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            bool pressed = (pinStatusMask & (static_cast<u32>(1) << ch)) != 0;
            handleChannel(ch, pressed, currentTick);
        }
    }

    KeyEvent getLastEvent(u8 channel) const {
        if (channel >= CHANNELS) {
            return KeyEvent::kNone;
        }
        return _state.lastEvent[channel];
    }

   private:
    void handleChannel(u8 channel, bool pressed, u32 tick) {
        switch (_state.state[channel]) {
            case KeyState::kNone:
                if (pressed) {
                    _state.pressTick[channel] = tick;
                    _state.state[channel]     = KeyState::kPress;
                    _state.lastEvent[channel] = KeyEvent::kPress;
                }
                break;

            case KeyState::kPress:
                if (pressed) {
                    if ((tick - _state.pressTick[channel]) > _config.holdThreshold) {
                        _state.state[channel]     = KeyState::kHold;
                        _state.lastEvent[channel] = KeyEvent::kHold;
                    }
                } else {
                    _state.state[channel]     = KeyState::kRelease;
                    _state.lastEvent[channel] = KeyEvent::kRelease;
                }
                break;

            case KeyState::kHold:
                if (!pressed) {
                    _state.state[channel]     = KeyState::kReleaseHold;
                    _state.lastEvent[channel] = KeyEvent::kRelease;
                }
                break;

            case KeyState::kRelease:
                _state.state[channel] = KeyState::kNone;

                // 根据时间间隔判断是否为连击
                KeyEvent clickEvent;
                if ((tick - _state.clickTick[channel]) < _config.clickIntervalThreshold &&
                    _state.clickTick[channel] != 0) {
                    // 在间隔内且有前置点击事件，判断为连击
                    clickEvent = KeyEvent::kClick2;
                } else {
                    // 首次点击或超过间隔，重新开始
                    clickEvent = KeyEvent::kClick;
                }

                _state.lastEvent[channel] = clickEvent;
                _state.clickTick[channel] = tick;
                break;

            case KeyState::kReleaseHold:
                _state.state[channel]     = KeyState::kNone;
                _state.clickTick[channel] = 0;  // 长按后重置，不算连击
                break;
        }
    }

   private:
    Config& _config;
    State   _state{};
};

}  // namespace wibot::dsp
