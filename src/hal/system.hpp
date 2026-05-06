#pragma once

#include "chip.hpp"
#include "type.hpp"

namespace wibot {

class System {
   public:
    static void delayUs(u32 us);
    [[deprecated("Use wibot::sleep instead.")]]
    static void delayMs(u32 ms);
    static u32  getTickMs();
    static u64  getTickUs();
    static u64  getTickNs();
    static u32  getDurationMs(u32 tick);

    static u32 getSysClockFreq();

    static u32 getHCLKFreq();

    static u32 getPCLK1Freq();

    static u32 getPCLK1TimFreq();

    static u32 getPCLK2Freq();

    static u32 getPCLK2TimFreq();

    static u32 getTIMFreq(TIM_TypeDef* tim);
};

class SystemTick {
   public:
    SystemTick() : _lastTickMs(System::getTickMs()) {
    }

    static u32 getTickMs() {
        return System::getTickMs();
    }
    u32 reset() {
        u32 currentTickMs = System::getTickMs();
        _lastTickMs       = currentTickMs;
        return _lastTickMs;
    }
    u32 getSamplePeriodMs() {
        u32 currentTickMs = System::getTickMs();
        u32 samplePeriod  = currentTickMs - _lastTickMs;
        _lastTickMs       = currentTickMs;
        return samplePeriod;
    }

   private:
    u32 _lastTickMs;
};

}  // namespace wibot
