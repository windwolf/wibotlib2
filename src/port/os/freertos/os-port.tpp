#pragma once

#include "cmsis_os2.h"
#include "os/os.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void runStub(void* instance);
#ifdef __cplusplus
}
#endif

namespace wibot {

inline Thread::Thread(const char* name, Worker& worker, u32 priority, u8* stackBuf, u16 stackSize,
                      const ThreadConfig& config) {
    ASSERT(stackBuf != nullptr, "Thread stack buffer must not be null.");
    ASSERT(stackSize > 0, "Thread stack size must be greater than 0.");
    _stack     = stackBuf;
    _stackSize = stackSize;

    osThreadAttr_t attr = {
        .name       = name,
        .attr_bits  = 0,
        .cb_mem     = &(_instance),
        .cb_size    = sizeof(_instance),
        .stack_mem  = _stack,
        .stack_size = _stackSize,
        .priority   = static_cast<osPriority_t>(priority),
        //.tz_module
    };

    auto rst = osThreadNew(static_cast<void (*)(void*)>(runStub), &worker, &attr);
    ASSERT(rst != NULL, "create Thread failed.")
};

inline Thread::~Thread() {
    _stack     = nullptr;
    _stackSize = 0;
};

inline void Thread::start() {
    osThreadResume(&_instance);
};

}  // namespace wibot
