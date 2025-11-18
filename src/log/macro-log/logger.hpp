#pragma once

//
// Created by zhouj on 2023/9/21.
//

// #include "project_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_NONE_    0
#define LOG_LEVEL_ERROR_   1
#define LOG_LEVEL_WARNING_ 2
#define LOG_LEVEL_INFO_    3
#define LOG_LEVEL_DEBUG_   4

#undef LOG_LEVEL__

#ifdef LOG_LEVEL_ERROR
#define LOG_LEVEL__ LOG_LEVEL_ERROR_
#else
#ifdef LOG_LEVEL_WARNING
#define LOG_LEVEL__ LOG_LEVEL_WARNING_
#else
#ifdef LOG_LEVEL_INFO
#define LOG_LEVEL__ LOG_LEVEL_INFO_
#else
#ifdef LOG_LEVEL_DEBUG
#define LOG_LEVEL__ LOG_LEVEL_DEBUG_
#else
#ifdef LOG_LEVEL_NONE
#define LOG_LEVEL__ LOG_LEVEL_NONE_
#else
#error "LOG_LEVEL MUST DEFINED"
#endif
#endif
#endif
#endif
#endif

#ifdef LOG_COLOR
#define LOG_CTRL_TEXT_BRIGHT_BLACK   "\x1B[1;30m"
#define LOG_CTRL_TEXT_BRIGHT_RED     "\x1B[1;31m"
#define LOG_CTRL_TEXT_BRIGHT_GREEN   "\x1B[1;32m"
#define LOG_CTRL_TEXT_BRIGHT_YELLOW  "\x1B[1;33m"
#define LOG_CTRL_TEXT_BRIGHT_BLUE    "\x1B[1;34m"
#define LOG_CTRL_TEXT_BRIGHT_MAGENTA "\x1B[1;35m"
#define LOG_CTRL_TEXT_BRIGHT_CYAN    "\x1B[1;36m"
#define LOG_CTRL_TEXT_BRIGHT_WHITE   "\x1B[1;37m"
#else
#define LOG_CTRL_TEXT_BRIGHT_BLACK
#define LOG_CTRL_TEXT_BRIGHT_RED
#define LOG_CTRL_TEXT_BRIGHT_GREEN
#define LOG_CTRL_TEXT_BRIGHT_YELLOW
#define LOG_CTRL_TEXT_BRIGHT_BLUE
#define LOG_CTRL_TEXT_BRIGHT_MAGENTA
#define LOG_CTRL_TEXT_BRIGHT_CYAN
#define LOG_CTRL_TEXT_BRIGHT_WHITE
#endif
#include "SEGGER_RTT.h"
#include "string.h"

#define LOGGER(name) [[maybe_unused]] static const char *LOG_NAME__ = name;

#define LOG(NAME, LEVEL, COLOR, FMT, ...) SEGGER_RTT_printf(0, COLOR LEVEL " %s " FMT "\n", NAME, ##__VA_ARGS__)

#if (LOG_LEVEL__ >= LOG_LEVEL_ERROR_)
#define LOG_E(FMT, ...) LOG(LOG_NAME__, "E", LOG_CTRL_TEXT_BRIGHT_RED, FMT, ##__VA_ARGS__)
#define LOG_E_INTERVAL(INTERVAL, FMT, ...) \
    {                                      \
        static u32 _log_count_ = 0;        \
        if (_log_count_ == 0) {            \
            LOG_E(FMT, ##__VA_ARGS__);     \
        }                                  \
        _log_count_++;                     \
        if (_log_count_ >= INTERVAL) {     \
            _log_count_ = 0;               \
        }                                  \
    }
#else
#define LOG_E(FMT, ...)
#define LOG_E_INTERVAL(INTERVAL, FMT, ...)
#endif

#if (LOG_LEVEL__ >= LOG_LEVEL_WARNING_)
#define LOG_W(FMT, ...) LOG(LOG_NAME__, "W", LOG_CTRL_TEXT_BRIGHT_YELLOW, FMT, ##__VA_ARGS__)
#define LOG_W_INTERVAL(INTERVAL, FMT, ...) \
    {                                      \
        static u32 _log_count_ = 0;        \
        if (_log_count_ == 0) {            \
            LOG_W(FMT, ##__VA_ARGS__);     \
        }                                  \
        _log_count_++;                     \
        if (_log_count_ >= INTERVAL) {     \
            _log_count_ = 0;               \
        }                                  \
    }
#else
#define LOG_W(FMT, ...)
#define LOG_W_INTERVAL(INTERVAL, FMT, ...)
#endif

#if (LOG_LEVEL__ >= LOG_LEVEL_INFO_)
#define LOG_I(FMT, ...) LOG(LOG_NAME__, "I", LOG_CTRL_TEXT_BRIGHT_BLACK, FMT, ##__VA_ARGS__)
#define LOG_I_INTERVAL(INTERVAL, FMT, ...) \
    {                                      \
        static u32 _log_count_ = 0;        \
        if (_log_count_ == 0) {            \
            LOG_I(FMT, ##__VA_ARGS__);     \
        }                                  \
        _log_count_++;                     \
        if (_log_count_ >= INTERVAL) {     \
            _log_count_ = 0;               \
        }                                  \
    }
#else
#define LOG_I(FMT, ...)
#define LOG_I_INTERVAL(INTERVAL, FMT, ...)
#endif

#if (LOG_LEVEL__ >= LOG_LEVEL_DEBUG_)
#define LOG_D(FMT, ...) LOG(LOG_NAME__, "D", LOG_CTRL_TEXT_BRIGHT_MAGENTA, FMT, ##__VA_ARGS__)
#define LOG_D_INTERVAL(INTERVAL, FMT, ...) \
    {                                      \
        static u32 _log_count_ = 0;        \
        if (_log_count_ == 0) {            \
            LOG_D(FMT, ##__VA_ARGS__);     \
        }                                  \
        _log_count_++;                     \
        if (_log_count_ >= INTERVAL) {     \
            _log_count_ = 0;               \
        }                                  \
    }
#else
#define LOG_D(FMT, ...)
#define LOG_D_INTERVAL(INTERVAL, FMT, ...)
#endif

#ifdef __cplusplus
}
#endif
