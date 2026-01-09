#pragma once

#include "cmsis_os.h"
#include "cmsis_os2.h"

namespace wibot::os {

#define LAST_BIT_MASK     0x00800000
#define LAST_TWO_BIT_MASK 0x00C00000

using THREAD_TYPEDEF                = StaticTask_t;
using THREAD_FUNCTION_ARGUMENT_TYPE = void*;

using MESSAGEQUEUE_TYPEDEF = StaticQueue_t;
using MUTEX_TYPEDEF        = StaticSemaphore_t;
using EVENTGROUP_TYPEDEF   = StaticEventGroup_t;
using SEMAPHORE_TYPEDEF    = StaticSemaphore_t;

#define TIMER_TYPEDEF \
    u32 _period;      \
    StaticTimer_t
using TIMER_FUNCTION_ARGUMENT_TYPE = void*;

struct ThreadConfig {};

}  // namespace wibot::os
