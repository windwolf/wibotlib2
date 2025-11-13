#pragma once

#include "cmsis_os2.h"
#include "os.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void runStub(void* instance);
#ifdef __cplusplus
}
#endif

namespace wibot {
template <u16 stack_size>
Thread<stack_size>::Thread(const char* name, Worker* worker, u32 priority,
                           const ThreadConfig& config) {
    ASSERT(worker != nullptr, "Thread worker is null.");

    osThreadAttr_t attr = {
        .name       = name,
        .attr_bits  = 0,
        .cb_mem     = &(_instance),
        .cb_size    = sizeof(_instance),
        .stack_mem  = _stack,
        .stack_size = stack_size,
        .priority   = static_cast<osPriority_t>(priority),
        //.tz_module
    };

    auto rst = osThreadNew(static_cast<void (*)(void*)>(runStub), worker, &attr);
    ASSERT(rst != NULL, "create Thread failed.")
};
template <u16 stack_size>
Thread<stack_size>::~Thread(){};

template <u16 stack_size>
void Thread<stack_size>::start() {
    osThreadResume(&_instance);
};

}  // namespace wibot