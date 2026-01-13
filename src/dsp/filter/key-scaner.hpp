#pragma once

#include "type.hpp"

namespace wibot {

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

/**
 * @brief 多通道按键扫描核心
 * 
 * 处理多个按键通道的事件检测，逻辑与 io 层保持一致：按下、长按、释放、单击、双击。
 * 完整的按键事件模式：
 * click once: press, release, click
 * click twice: press, release, click, press, release, click2.
 * hold: press, hold, release
 * click then hold: press, release, click, press, hold, release
 *
 * @note 事件仅在发生的那一刻生成, 下一轮扫描会清除事件状态.
 *       因此需要及时读取事件状态. 

 * @tparam CHANNELS 按键通道数量
 */
template <u8 CHANNELS>
class KeyScaner {
   public:
    struct Config {
        u16 holdThreshold{500};           // 长按阈值 (ms)
        u16 clickIntervalThreshold{300};  // 连击间隔阈值 (ms)，为0则禁用连击检测
    };

    struct State {
        u16      pressTimer[CHANNELS]{};  // 按下累积时间 (ms)
        u16      clickTimer[CHANNELS]{};  // 点击间隔累积时间 (ms)
        KeyEvent currentEvent[CHANNELS]{KeyEvent::kNone};
        KeyState state[CHANNELS]{KeyState::kNone};
    };

   public:
    explicit KeyScaner(Config& config) : _config(config) {
    }

    void reset() {
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            _state.pressTimer[ch]   = 0;
            _state.clickTimer[ch]   = 0;
            _state.currentEvent[ch] = KeyEvent::kNone;
            _state.state[ch]        = KeyState::kNone;
        }
    }

    /**
     * @brief 处理按键输入（多通道位掩码）
     *
     * @param pinStatusMask    位掩码，bit=1 表示对应通道按下
     * @param samplePeriodMs   当前采样周期 (ms)
     */
    void scan(u32 pinStatusMask, u32 samplePeriodMs) {
        // 清除上一轮的事件状态，保证事件仅在发生的那一刻生成
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            _state.currentEvent[ch] = KeyEvent::kNone;
        }

        // 处理本轮按键输入
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            bool pressed = (pinStatusMask & (static_cast<u32>(1) << ch)) != 0;
            handleChannel(ch, pressed, samplePeriodMs);
        }
    }

    KeyEvent getCurrentEvent(u8 channel) const {
        if (channel >= CHANNELS) {
            return KeyEvent::kNone;
        }
        return _state.currentEvent[channel];
    }

   private:
    void handleChannel(u8 channel, bool pressed, u32 samplePeriodMs) {
        switch (_state.state[channel]) {
            case KeyState::kNone:
                if (pressed) {
                    _state.pressTimer[channel]   = 0;
                    _state.state[channel]        = KeyState::kPress;
                    _state.currentEvent[channel] = KeyEvent::kPress;
                } else {
                    // 累积点击间隔时间，用于连击检测
                    u32 newClickTimer = _state.clickTimer[channel] + samplePeriodMs;
                    if (newClickTimer < 65536) {  // 防止 u16 溢出
                        _state.clickTimer[channel] = static_cast<u16>(newClickTimer);
                    }
                }
                break;

            case KeyState::kPress:
                if (pressed) {
                    u32 newTimer = _state.pressTimer[channel] + samplePeriodMs;
                    if (newTimer > _config.holdThreshold) {
                        _state.state[channel]        = KeyState::kHold;
                        _state.currentEvent[channel] = KeyEvent::kHold;
                        _state.pressTimer[channel]   = 0;
                    } else {
                        _state.pressTimer[channel] = static_cast<u16>(newTimer);
                    }
                } else {
                    _state.state[channel]        = KeyState::kRelease;
                    _state.currentEvent[channel] = KeyEvent::kRelease;
                }
                break;

            case KeyState::kHold:
                if (!pressed) {
                    _state.state[channel]        = KeyState::kReleaseHold;
                    _state.currentEvent[channel] = KeyEvent::kRelease;
                }
                break;

            case KeyState::kRelease:
                _state.state[channel] = KeyState::kNone;

                // 根据时间间隔判断是否为连击；阈值为0则禁用连击检测
                KeyEvent clickEvent;
                if (_config.clickIntervalThreshold == 0) {
                    clickEvent                 = KeyEvent::kClick;
                    _state.clickTimer[channel] = 0;
                } else if (_state.clickTimer[channel] < _config.clickIntervalThreshold &&
                           _state.clickTimer[channel] != 0) {
                    // 在间隔内且有前置点击事件，判断为连击
                    clickEvent                 = KeyEvent::kClick2;
                    _state.clickTimer[channel] = 0;
                } else {
                    // 首次点击或超过间隔，重新开始
                    clickEvent                 = KeyEvent::kClick;
                    _state.clickTimer[channel] = 0;
                }

                _state.currentEvent[channel] = clickEvent;
                break;

            case KeyState::kReleaseHold:
                _state.state[channel]      = KeyState::kNone;
                _state.clickTimer[channel] = 0;  // 长按后重置，不算连击
                break;
        }
    }

   private:
    Config& _config;
    State   _state{};
};

}  // namespace wibot
