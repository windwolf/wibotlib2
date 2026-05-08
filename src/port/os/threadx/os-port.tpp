#pragma once

#include "os/os.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void runStub(ULONG instance);
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

    auto preemptionThreshold = config.preemptionThreshold;
    if (preemptionThreshold == 0) {
        preemptionThreshold = priority;
    }
    auto rst = tx_thread_create(&_instance, const_cast<CHAR*>(name), runStub,
                                reinterpret_cast<ULONG>(&worker), _stack, _stackSize, priority,
                                preemptionThreshold, config.timeSlice, TX_DONT_START);
    ASSERT(rst == TX_SUCCESS, "create Thread failed.");
};

inline Thread::~Thread() {
    tx_thread_delete(&_instance);
    _stack     = nullptr;
    _stackSize = 0;
};

inline void Thread::start() {
    tx_thread_resume(&_instance);
};

}  // namespace wibot
