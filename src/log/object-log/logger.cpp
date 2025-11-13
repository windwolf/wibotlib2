////
//// Created by zhouj on 2023/9/9.
////
//
//#include "Logger.hpp"
//
//#include <cstdarg>
//#include <cstdio>
//#include <cstring>
//
//namespace wibot {
//
//#define LOG_CTRL_TEXT_BRIGHT_BLACK   "\x1B[1;30m"
//#define LOG_CTRL_TEXT_BRIGHT_RED     "\x1B[1;31m"
//#define LOG_CTRL_TEXT_BRIGHT_GREEN   "\x1B[1;32m"
//#define LOG_CTRL_TEXT_BRIGHT_YELLOW  "\x1B[1;33m"
//#define LOG_CTRL_TEXT_BRIGHT_BLUE    "\x1B[1;34m"
//#define LOG_CTRL_TEXT_BRIGHT_MAGENTA "\x1B[1;35m"
//#define LOG_CTRL_TEXT_BRIGHT_CYAN    "\x1B[1;36m"
//#define LOG_CTRL_TEXT_BRIGHT_WHITE   "\x1B[1;37m"
//
//#define LOG(MODULE, LEVEL, COLOR, FMT, ...) \
//    printf(COLOR "[" LEVEL "] [" MODULE "] " FMT "\r\n", ##__VA_ARGS__)
//
//Logger& Logger::GetGlobalInstance() {
//    static auto logger = Logger("wibot");
//    return logger;
//}
//
//void Logger::RegisterInstance(Logger* instance) {
//    GetGlobalInstance().Append(instance);
//}
//
//void Logger::UnregisterInstance(Logger* instance) {
//    GetGlobalInstance().Remove(instance);
//}
//
//Logger::Logger(const char* name) : name_(name) {
//    Logger::RegisterInstance(this);
//}
//Logger::~Logger() {
//    Logger::UnregisterInstance(this);
//}
//void Logger::SetGlobalLevel(LogLevel level) {
//    auto ins = &GetGlobalInstance();
//    while (ins != nullptr) {
//        ins->level_ = level;
//        ins         = static_cast<Logger*>(ins->next_);
//    }
//}
//void Logger::SetLevel(const char* name, LogLevel level) {
//    auto ins = &GetGlobalInstance();
//    while (ins != nullptr) {
//        if (strcmp(name, ins->name_) == 0) {
//            ins->level_ = level;
//        }
//        ins = static_cast<Logger*>(ins->next_);
//    }
//}
//
//void Logger::Error(const char* fmt, ...) {
//    va_list args;
//    va_start(args, fmt);
//    printf(LOG_CTRL_TEXT_BRIGHT_RED "[E] [%s] ", name_);
//    printf(fmt, args);
//    printf("\r\n");
//    va_end(args);
//}
//void Logger::Warning(const char* fmt, ...) {
//    va_list args;
//    va_start(args, fmt);
//    printf(LOG_CTRL_TEXT_BRIGHT_YELLOW "[W] [%s] ", name_);
//    printf(fmt, args);
//    printf("\r\n");
//    va_end(args);
//}
//void Logger::Info(const char* fmt, ...) {
//    va_list args;
//    va_start(args, fmt);
//    printf(LOG_CTRL_TEXT_BRIGHT_WHITE "[I] [%s] ", name_);
//    printf(fmt, args);
//    printf("\r\n");
//    va_end(args);
//}
//void Logger::Debug(const char* fmt, ...) {
//    va_list args;
//    va_start(args, fmt);
//    printf(LOG_CTRL_TEXT_BRIGHT_MAGENTA "[D] [%s] ", name_);
//    printf(fmt, args);
//    printf("\r\n");
//    va_end(args);
//}
//
//}  // namespace wibot
