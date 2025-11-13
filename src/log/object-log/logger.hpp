#pragma once

//
// Created by zhouj on 2023/9/9.
//

#include "type.hpp"

namespace wibot {

enum class LogLevel : u8 {
    kError   = 0x01,
    kWarning = 0x02,
    kInfo    = 0x04,
    kDebug   = 0x08,

    kNone             = 0x00,
    kErrorWarning     = 0x03,
    KErrorWarningInfo = 0x07,
    kAll              = 0xFF,
};

class Logger : protected LinkList {
   public:
    static void    SetGlobalLevel(LogLevel level);
    static void    SetLevel(const char* name, LogLevel level);
    static Logger& GetGlobalInstance();

   private:
    static void RegisterInstance(Logger* instance);
    static void UnregisterInstance(Logger* instance);

   public:
    explicit Logger(const char* name);
    ~Logger();
    void Error(const char* fmt, ...);
    void Warning(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Debug(const char* fmt, ...);

   private:
    const char* name_;
    LogLevel    level_ = LogLevel::kAll;
};

#define LOGGER_WRAP(loggerInstance, name) \
    class loggerInstance {                \
       public:                            \
        static Logger& GetInstance() {    \
            static Logger logger(name);   \
            return logger;                \
        };                                \
    };
#define LOGGER_UNWRAP(loggerInstance) loggerInstance::GetInstance()

#define LOG_E(logger, fmt, ...) logger.Error(fmt, ##__VA_ARGS__)
#define LOG_E_INTERVAL(logger, INTERVAL, fmt, ...) \
    {                                              \
        static u32 i = 0;                          \
        if ((i % INTERNAL) == 0) {                 \
            LOG_E(FMT, ##__VA_ARGS__);             \
        }                                          \
        i++;                                       \
    }

#define LOG_W(logger, fmt, ...) logger.Warning(fmt, ##__VA_ARGS__)
#define LOG_W_INTERVAL(logger, INTERVAL, fmt, ...) \
    {                                              \
        static u32 i = 0;                          \
        if ((i % INTERNAL) == 0) {                 \
            LOG_E(FMT, ##__VA_ARGS__);             \
        }                                          \
        i++;                                       \
    }

#define LOG_I(logger, fmt, ...) logger.Info(fmt, ##__VA_ARGS__)
#define LOG_I_INTERVAL(logger, INTERVAL, fmt, ...) \
    {                                              \
        static u32 i = 0;                          \
        if ((i % INTERNAL) == 0) {                 \
            LOG_E(FMT, ##__VA_ARGS__);             \
        }                                          \
        i++;                                       \
    }

#define LOG_D(logger, fmt, ...) logger.Debug(fmt, ##__VA_ARGS__)
#define LOG_D_INTERVAL(logger, INTERVAL, fmt, ...) \
    {                                              \
        static u32 i = 0;                          \
        if ((i % INTERNAL) == 0) {                 \
            LOG_E(FMT, ##__VA_ARGS__);             \
        }                                          \
        i++;                                       \
    }
}  // namespace wibot
