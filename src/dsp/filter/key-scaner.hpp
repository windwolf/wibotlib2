#pragma once

#include "type.hpp"

namespace wibot {

enum class KeyEvent : u8 {
    kNone    = 0,
    kPress   = 1,  // 按下
    kHold    = 2,  // 长按
    kRelease = 3,  // 释放
    kClick   = 4,  // 单击（释放后）
    kClick2  = 5,  // 连击2次
    kClick3  = 6,  // 连击3次
};

/**
 * @brief 多通道按键扫描核心
 * 按键动作和事件时序如下:
 * _表示按钮弹起状态, ‾表示按钮按下状态. -表示无事件, 1表示Press, 2表示Hold, 3表示Release, 4表示Click, 5表示Click2.
 * 1. 单击(click once): 按下, (小于保持阈值时间内释放)单击. 如下图所示:
 * __ˉˉ¯___
 *   1  4
 * 2. 双击(click twice): 按下, (小于保持阈值时间内释放)单击, (小于连击阈值内)再次按下, (小于保持阈值时间内释放)连击2. 如下图所示:
 * __ˉˉ¯____ˉ¯ˉ¯_____
 *   1  4   1   5    
 * 3. 长按(hold): 按下, (超过保持阈值时间)长按, 释放. 如下图所示:
 * __ˉˉˉˉ|ˉˉˉˉ¯_____
 *  1    2     3
 * 4. 两次独立的单击: 按下, (小于保持阈值时间内释放)单击, (超过连击阈值时间)再次按下, (小于保持阈值时间内释放)单击. 如下图所示:
 * __ˉˉ¯______|__ˉˉ¯_____
 *  1   4        1  4
 * @note 事件仅在发生的那一刻生成, 下一轮扫描会清除事件状态.
 *       因此需要及时读取事件状态. 

 * @tparam CHANNELS 按键通道数量
 */
template <u8 CHANNELS>
class KeyScaner {
   public:
    enum KeyState {
        kNone  = 0,
        kPress = 1,
        kHold  = 2,
    };

    union KeyStatus {
        struct {
            KeyState state      : 3;
            KeyEvent event      : 3;
            u8       clickCount : 2;  // 0~2
        };
        u32 raw;
    };

   public:
    struct Config {
        u16 holdThreshold{500};           // 长按阈值 (ms)
        u16 clickIntervalThreshold{300};  // 连击间隔阈值 (ms)，为0则禁用连击检测
    };

    struct State {
        u16       holdingTimer[CHANNELS]{};  // 按下累积时间 (ms)
        u16       clickTimer[CHANNELS]{};    // 点击间隔累积时间 (ms)
        KeyStatus state[CHANNELS]{};
    };

   public:
    explicit KeyScaner(Config& config) : _config(config) {
    }

    void reset() {
        for (u8 ch = 0; ch < CHANNELS; ++ch) {
            _state.holdingTimer[ch] = 0;
            _state.clickTimer[ch]   = 0;
            _state.state[ch]        = {.raw = 0};
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
            _state.state[ch].event = KeyEvent::kNone;
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
        return _state.state[channel].event;
    }

   private:
    void handleChannel(u8 channel, bool pressed, u32 samplePeriodMs) {
        auto& state = _state.state[channel];

        switch (state.state) {
            case KeyState::kNone: {
                // 检查连击间隔阈值，超时则清除连击计数
                if (state.clickCount > 0) {
                    _state.clickTimer[channel] += samplePeriodMs;
                    if (_config.clickIntervalThreshold > 0 &&
                        _state.clickTimer[channel] >= _config.clickIntervalThreshold) {
                        state.clickCount           = 0;
                        _state.clickTimer[channel] = 0;
                    }
                }

                // 新按下事件
                if (pressed) {
                    _state.holdingTimer[channel] = 0;
                    state.state                  = KeyState::kPress;
                    state.event                  = KeyEvent::kPress;
                }
                break;
            }

            case KeyState::kPress: {
                // 累积按下时间
                _state.holdingTimer[channel] += samplePeriodMs;

                // 检查是否超过长按阈值
                if (_state.holdingTimer[channel] >= _config.holdThreshold) {
                    // 如果超时未松开, 生成Hold事件; 如果超时松开(边界条件), 也要生成Hold事件, Release事件在下一轮处理;
                    state.state = KeyState::kHold;
                    state.event = KeyEvent::kHold;
                } else {
                    if (!pressed) {
                        // 短按释放
                        if (_config.clickIntervalThreshold == 0) {
                            // 不支持连击，直接生成单击事件
                            state.event      = KeyEvent::kClick;
                            state.clickCount = 0;
                        } else {
                            if (state.clickCount == 0) {
                                state.event      = KeyEvent::kClick;
                                state.clickCount = 1;
                            } else if (state.clickCount == 1) {
                                state.event      = KeyEvent::kClick2;
                                state.clickCount = 2;
                            } else if (state.clickCount == 2) {
                                state.event      = KeyEvent::kClick3;
                                state.clickCount = 0;
                            }
                        }
                        state.state                = KeyState::kNone;
                        _state.clickTimer[channel] = 0;
                    }
                }
                break;
            }

            case KeyState::kHold: {
                if (!pressed) {
                    // 长按释放
                    state.state                = KeyState::kNone;
                    state.event                = KeyEvent::kRelease;
                    state.clickCount           = 0;
                    _state.clickTimer[channel] = 0;
                }
                break;
            }
        }
    }

   private:
    Config& _config;
    State   _state{};
};

}  // namespace wibot
