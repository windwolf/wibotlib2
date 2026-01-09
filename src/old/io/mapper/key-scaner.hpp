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
   protected:
    enum class KeyState : u8 {
        kNone,
        kPress,
        kHold,
        kRelease,
        kReleaseHold,
    };

   public:
    struct Storage {
        u32      pressTick[CHANNELS]{};
        u32      clickTick[CHANNELS]{};
        KeyEvent lastEvent[CHANNELS]{KeyEvent::kNone};
        KeyState state[CHANNELS]{KeyState::kNone};
        u8       clickCount[CHANNELS]{};
    };

   public:
    KeyScaner(DigitalSource<CHANNELS>& upstream, KeyScanerConfig& config, Storage& storage)
        : _upstream(upstream), _config(config), _storage(storage) {
        reset();
    }

    void update() override {
        _upstream.update();
        auto _pinStatus = _upstream.getValues();
        auto now        = System::getTickMs();

        // Update digital input and get current pin status

        for (u8 channel = 0; channel < CHANNELS; channel++) {
            // Get the current pin status for this channel
            bool currentPinStatus = (_pinStatus & (1U << channel));
            auto state            = _storage.state[channel];
            switch (state) {
                case KeyState::kNone:
                    if (currentPinStatus) {
                        _storage.pressTick[channel] = now;
                        _storage.state[channel]     = KeyState::kPress;
                        _storage.lastEvent[channel] = KeyEvent::kPress;
                    }
                    break;
                case KeyState::kPress:
                    if (currentPinStatus) {
                        if ((now - _storage.pressTick[channel]) > _config.holdThreshold) {
                            _storage.state[channel]     = KeyState::kHold;
                            _storage.lastEvent[channel] = KeyEvent::kHold;
                        }
                    } else {
                        _storage.state[channel]     = KeyState::kRelease;
                        _storage.lastEvent[channel] = KeyEvent::kRelease;
                    }
                    break;
                case KeyState::kHold:
                    if (!currentPinStatus) {
                        _storage.state[channel]     = KeyState::kReleaseHold;
                        _storage.lastEvent[channel] = KeyEvent::kRelease;
                    }
                    break;
                case KeyState::kRelease:
                    if ((now - _storage.clickTick[channel]) < _config.clickIntervalThreshold) {
                        _storage.clickCount[channel]++;
                    } else {
                        _storage.clickCount[channel] = 1;
                    }
                    _storage.clickTick[channel] = now;
                    _storage.state[channel]     = KeyState::kNone;
                    _storage.lastEvent[channel] = KeyEvent::kClick;
                    break;
                case KeyState::kReleaseHold:
                    _storage.state[channel]      = KeyState::kNone;
                    _storage.clickCount[channel] = 0;
                    break;
            }
        }
    }

    void reset() override {
        for (u8 channel = 0; channel < CHANNELS; channel++) {
            _storage.pressTick[channel]  = 0;
            _storage.clickTick[channel]  = 0;
            _storage.lastEvent[channel]  = KeyEvent::kNone;
            _storage.state[channel]      = KeyState::kNone;
            _storage.clickCount[channel] = 0;
        }
    }

    /**
     * @brief 获取指定通道的按键事件
     */
    KeyEvent getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return KeyEvent::kNone;
        }
        return _storage.lastEvent[channel];
    }

    /**
     * @brief 获取指定通道的点击次数
     */
    u8 getClickCount(u8 channel) const {
        if (channel >= CHANNELS) {
            return 0;
        }
        return _storage.clickCount[channel];
    }

   private:
    DigitalSource<CHANNELS>& _upstream;
    KeyScanerConfig&         _config;
    Storage&                 _storage;
};

}  // namespace wibot
