
#include "cmsis_os2.h"
#include "os/os.hpp"
#include "hal/system.hpp"
#include "logger.hpp"
LOGGER("os")

#ifdef __cplusplus
extern "C" {
#endif
void runStub(void* instance) {
    static_cast<wibot::os::Worker*>(instance)->run();
};
#ifdef __cplusplus
}
#endif

namespace wibot::os {

static const Result OsStatusToResult(osStatus_t status) {
    switch (status) {
        case osOK:
            return Result::kOk;
        case osError:
            return Result::kError;
        case osErrorTimeout:
            return Result::kTimeout;
        case osErrorResource:
            return Result(Result::ResultStatus::kNoResource, status);
        case osErrorParameter:
            return Result::kInvalidParameter;
        case osErrorNoMemory:
            return Result(Result::ResultStatus::kNoResource, status);
        case osErrorISR:
            return Result(Result::ResultStatus::kError, status);
        case osStatusReserved:
            return Result(Result::ResultStatus::kError, status);
        default:
            return Result(Result::ResultStatus::kError, status);
    }
}

void sleep(u32 ms) {
    switch (getContextMode()) {
        case ContextMode::kThread:
            osDelay(ms);
            break;
        case ContextMode::kISR:
            ASSERT(false, "Cannot call sleep in ISR context.");
            return;
        case ContextMode::kInit:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
            hal::System::delayMs(ms);
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
            return;
        default:
            ASSERT(false, "Unknown context mode.");
            return;
    }
};

bool isInThread() {
    if (osKernelGetState() == osKernelState_t::osKernelRunning && osThreadGetId() != NULL) {
        return true;
    }
    return false;
}

Mutex::Mutex(const char* name) {
    osMutexAttr_t attr = {
        .name    = name,
        .cb_mem  = &(_instance),
        .cb_size = sizeof(_instance),
    };

    auto rst = osMutexNew(&attr);
    ASSERT(rst != NULL, "create mutex failed.")
};

Mutex::~Mutex() {
    osMutexDelete(&(_instance));
};

Result Mutex::lock(u32 timeout) {
    auto rst = osMutexAcquire(&(_instance), timeout);
    return OsStatusToResult(rst);
};

void Mutex::unlock() {
    osMutexRelease(&(_instance));
}

EventGroup::EventGroup(const char* name) {
    osEventFlagsAttr_t attr = {
        .name    = name,
        .cb_mem  = &(_instance),
        .cb_size = sizeof(_instance),
    };

    auto rst = osEventFlagsNew(&attr);
    ASSERT(rst != NULL, "create eventFlags failed.")
}
EventGroup::~EventGroup() {
    osEventFlagsDelete(&(_instance));
};

Result EventGroup::set(u32 flags) {
    auto rst = osEventFlagsSet(&(_instance), flags);
    if ((i32)rst < 0) {
        return OsStatusToResult(static_cast<osStatus_t>(rst));
    } else {
        return Result::kOk;
    }
};

Result EventGroup::reset(u32 flags) {
    auto rst = osEventFlagsClear(&(_instance), flags);
    if ((i32)rst < 0) {
        return OsStatusToResult(static_cast<osStatus_t>(rst));
    } else {
        return Result::kOk;
    }
};

Result EventGroup::wait(u32 flags, u32& actual_flags, EventOptions options, u32 timeout) {
    u32 freertosOptions = 0;
    if ((options & EventOptions_ClearFlag) == EventOptions_NoClear) {
        freertosOptions |= osFlagsNoClear;
    }
    if ((options & EventOptions_WaitFlag) == EventOptions_WaitForAll) {
        freertosOptions |= osFlagsWaitAll;
    }
    // TODO: HANDLE options
    auto rst = osEventFlagsWait(&(_instance), flags, freertosOptions, timeout);
    if ((i32)rst < 0) {
        return OsStatusToResult(static_cast<osStatus_t>(rst));
    } else {
        actual_flags = rst;
        return Result::kOk;
    }
};

MessageQueue::MessageQueue(const char* name, void* msgAddr, u32 msgSize, u32 queueSize) {
    osMessageQueueAttr_t attr = {
        .name    = name,
        .cb_mem  = &(_instance),
        .cb_size = sizeof(_instance),
        .mq_mem  = msgAddr,
        .mq_size = msgSize * queueSize * 4,
    };
    auto rst = osMessageQueueNew(queueSize, msgSize * 4, &attr);
    ;
    ASSERT(rst != NULL, "create messagequeue failed.")
}

MessageQueue::~MessageQueue() {
    osEventFlagsDelete(&(_instance));
}

OsTimer::OsTimer(const char* name, Worker& worker, u32 period, u32 firstDelay) : _period(period) {
    osTimerAttr_t attr = {
        .name = name, .attr_bits = 0, .cb_mem = &(_instance), .cb_size = sizeof(_instance),
        //.tz_module
    };

    auto rst = osTimerNew(runStub, osTimerPeriodic, &worker, &attr);
    ASSERT(rst != NULL, "create OsTimer failed.")
}
OsTimer::~OsTimer() {
    osTimerDelete(&_instance);
}

void OsTimer::start() {
    osTimerStart(&_instance, _period);
}

void OsTimer::stop() {
    osTimerStop(&_instance);
}

}  // namespace wibot
