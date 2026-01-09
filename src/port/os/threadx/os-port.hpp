#pragma once

#include "tx_api.h"

namespace wibot::os {

#define LAST_BIT_MASK     0x80000000
#define LAST_TWO_BIT_MASK 0xC0000000

using THREAD_TYPEDEF                = TX_THREAD;
using THREAD_FUNCTION_ARGUMENT_TYPE = ULONG;

using MESSAGEQUEUE_TYPEDEF = TX_QUEUE;
using MUTEX_TYPEDEF        = TX_MUTEX;
using EVENTGROUP_TYPEDEF   = TX_EVENT_FLAGS_GROUP;
using SEMAPHORE_TYPEDEF    = TX_SEMAPHORE;

using TIMER_TYPEDEF                = TX_TIMER;
using TIMER_FUNCTION_ARGUMENT_TYPE = ULONG;

struct ThreadConfig {
    ThreadConfig() : preemptionThreshold(0), timeSlice(0) {};
    UINT preemptionThreshold;
    UINT timeSlice;
};

}  // namespace wibot::os
