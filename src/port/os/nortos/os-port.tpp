#pragma once

#include "os/os.hpp"

namespace wibot {

template <u16 stack_size>
inline Thread<stack_size>::Thread(const char* name, Worker& worker, u32 priority,
                                  const ThreadConfig& config)
    : _instance(&worker) {
    (void)name;
    (void)priority;
    (void)config;
    static_assert(stack_size > 0, "Thread stack size must be greater than 0.");
};

template <u16 stack_size>
inline Thread<stack_size>::~Thread() {
};

template <u16 stack_size>
inline void Thread<stack_size>::start() {
    _instance->run();
};

}  // namespace wibot
