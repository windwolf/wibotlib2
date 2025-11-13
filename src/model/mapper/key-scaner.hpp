#pragma once

//
// Created by zhouj on 2024/3/25.
//
#include "model.hpp"
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
    KeyScaner(SyncPipeline<bool, u32>& upstream, u16 holdThreshold = 1000,
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

}  // namespace wibot
