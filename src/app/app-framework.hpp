#pragma once

//
// Created by zhouj on 2023/9/13.
//

#
#include "logger.hpp"
#include "type.hpp"

#ifdef USE_RTT_PRINT
#include "SEGGER_RTT.h"
#endif

namespace wibot {

template <typename T>
class BootFrom {
   public:
    static T   *app;
    static void boot(char const *name, char const *version) {
#ifdef USE_RTT_PRINT
        SEGGER_RTT_Init();
#endif
        LOGGER("app");
        LOG_I("App name: %s, version: %s", name, version);
        static T _app;
        app = &_app;
        _app.boot();
    }
};

template <typename T>
T *BootFrom<T>::app = nullptr;

};  // namespace wibot
