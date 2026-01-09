#pragma once

#include "chip.hpp"
#include "type.hpp"

namespace wibot::hal {

class System {
   public:
    static void delayUs(u32 us);
    [[deprecated("Use wibot::os::sleep instead.")]]
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

}  // namespace wibot
