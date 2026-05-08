#pragma once

#include "os/os.hpp"

namespace wibot {

inline Thread::Thread(const char* name, Worker& worker, u32 priority, u8* stackBuf, u16 stackSize,
                      const ThreadConfig& config)
    : _instance(&worker) {
    (void)name;
    (void)priority;
    (void)config;
    ASSERT(stackBuf != nullptr, "Thread stack buffer must not be null.");
    ASSERT(stackSize > 0, "Thread stack size must be greater than 0.");
    _stack     = stackBuf;
    _stackSize = stackSize;
};

inline Thread::~Thread() {
    _stack     = nullptr;
    _stackSize = 0;
};

inline void Thread::start() {
    _instance->run();
};

}  // namespace wibot
