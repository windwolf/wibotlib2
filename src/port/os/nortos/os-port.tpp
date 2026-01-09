#pragma once

#include "os/os.hpp"

namespace wibot::os {
template <u16 stack_size>
Thread<stack_size>::Thread(const char* name, Worker& worker, u32 priority,
                           const ThreadConfig& config)
    : _instance(&worker) {
    ASSERT(worker != nullptr, "Thread worker is null.");
};
template <u16 stack_size>
Thread<stack_size>::~Thread(){

};

template <u16 stack_size>
void Thread<stack_size>::start() {
    _instance->run();
};

}  // namespace wibot::os