#pragma once

#include "chip.hpp"

namespace wibot {

class System {
   public:
    static void delayUs(u32 us);
    static void delayMs(u32 ms);
    static u32  getTickMs();
    // static u64  getTickUs();
    // static u64  getTickNs();
    static u32  getDurationMs(u32 tick);
};

}  // namespace wibot
