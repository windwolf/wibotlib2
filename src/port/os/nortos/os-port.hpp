#pragma once

#include "circular-buffer.hpp"

namespace wibot::os {

#define LAST_BIT_MASK     0x80000000
#define LAST_TWO_BIT_MASK 0xC0000000

#define THREAD_TYPEDEF wibot::Worker*
using THREAD_FUNCTION_ARGUMENT_TYPE = u32;

#define MESSAGEQUEUE_TYPEDEF \
    uint32_t _msgSize;       \
    CircularBuffer8
using MUTEX_TYPEDEF      = bool;
using EVENTGROUP_TYPEDEF = u32;
using SEMAPHORE_TYPEDEF  = u32;

#define TIMER_TYPEDEF wibot::Worker*
using TIMER_FUNCTION_ARGUMENT_TYPE = u32;

struct ThreadConfig {};

}  // namespace wibot
