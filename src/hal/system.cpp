//
// Created by zhouj on 2023/9/9.
//
#include "system.hpp"

namespace wibot::hal {
u32 System::getDurationMs(u32 tick) {
    return getTickMs() - tick;
}

}  // namespace wibot::hal
