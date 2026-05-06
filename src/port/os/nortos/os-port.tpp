#pragma once

#include "os/os.hpp"

#include <cstdlib>

namespace wibot {

inline Thread::Thread(const char* name, Worker& worker, u32 priority,
                      const ThreadConfig& config)
    : Thread(name, worker, priority, kDefaultStackSize, config) {
}

inline Thread::Thread(const char* name, Worker& worker, u32 priority, u16 stackSize,
                      const ThreadConfig& config)
    : _instance(&worker) {
    (void)name;
    (void)priority;
    (void)config;
    ASSERT(stackSize > 0, "Thread stack size must be greater than 0.");
    _stack = static_cast<u8*>(std::malloc(stackSize));
    ASSERT(_stack != nullptr, "allocate Thread stack failed.");
    _stackSize = stackSize;
};

inline Thread::~Thread(){
    std::free(_stack);
    _stack     = nullptr;
    _stackSize = 0;
};

inline void Thread::start() {
    _instance->run();
};

}  // namespace wibot
