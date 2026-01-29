#pragma once

//
// Created by zhouj on 2023/9/13.
//

#include "logger.hpp"
#include "type.hpp"

#ifdef USE_RTT_PRINT
#include "log/rtt/SEGGER_RTT.h"
#endif

#define BOOT_ENTRY __attribute__((section(".bss.lateinit")))

namespace wibot {

template <typename T>
union BootFrom {
   public:
    BootFrom() {};
    ~BootFrom() {};

    void boot(char const *name, char const *version) {
#ifdef USE_RTT_PRINT
        SEGGER_RTT_Init();
#endif
        LOGGER("app");
        LOG_I("App name: %s, version: %s", name, version);
        new (_storage) T();
        _object.boot();
    }

   private:
    // 类型化成员（调试器可见，指向同一块内存）
    T _object;
    // 原始内存（placement new 会在此处构造）
    // 初始状态：未初始化的原始字节
    alignas(T) unsigned char _storage[sizeof(T)];
};

};  // namespace wibot
