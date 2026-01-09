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
template <u16 stack_size>
Thread<stack_size>::Thread(const char* name, Worker& worker, u32 priority,
                           const ThreadConfig& config) {
    auto preemptionThreshold = config.preemptionThreshold;
    if (preemptionThreshold == 0) {
        preemptionThreshold = priority;
    }
    auto rst = tx_thread_create(&_instance, const_cast<CHAR*>(name), runStub,
                                reinterpret_cast<ULONG>(&worker), _stack, stack_size, priority,
                                preemptionThreshold, config.timeSlice, TX_DONT_START);
    ASSERT(rst == TX_SUCCESS, "create Thread failed.");
};
template <u16 stack_size>
Thread<stack_size>::~Thread() {
    tx_thread_delete(&_instance);
};

template <u16 stack_size>
void Thread<stack_size>::start() {
    tx_thread_resume(&_instance);
};

}  // namespace wibot