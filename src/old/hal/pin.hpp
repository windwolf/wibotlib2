#pragma once

#include "type.hpp"

namespace wibot {

PIN_PER_DECL
enum class PinStatus : bool {
    kReset = false,
    kSet   = true,
};

enum class PinMode : u8 {
    kInput  = 0U,
    kOutput = 1U,
};
union PinConfig {
    struct {
        bool inverse    : 1;
        u8              : 7;
        /**
         * Debounce time in ms. 0 means no debounce.
         */
        u8 debounceTime : 8;
        u32             : 16;
    };
    u32 value;
};

class Pin {
   public:
    Pin(PIN_CTOR_ARG, u16 pinMask);

    PinStatus read();
    void      write(PinStatus value);

    Result toggle();
    Result setMode(PinMode mode);

   public:
    PinConfig config;

   private:
    PIN_FIELD_DECL;
    u16       _pinMask;
    PinStatus _lastOutputStatus   = PinStatus::kReset;
    PinStatus _lastBufferedStatus = PinStatus::kReset;
    u32       _lastDebounceTime   = 0;
    bool      _isFirstValue       = true;
};
}  // namespace wibot
