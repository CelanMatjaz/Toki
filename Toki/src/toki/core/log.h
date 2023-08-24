#pragma once

#include "string"

namespace Toki {

    enum class LogLevel {
        Info,
        Warn,
        Error,
        Fatal,
        Debug,
        Trace
    };

    template<typename ...Args>
    void Log(LogLevel logLevel, std::string tmp, Args&& ...args);

}

#define TK_LOG_INFO(tmp, ...) Toki::Log(LogLevel::Info, tmp __VA_OPT__(,) ##__VA_ARGS__)
#define TK_LOG_WARN(tmp, ...) Toki::Log(LogLevel::Warn, tmp __VA_OPT__(,)__VA_ARGS__)
#define TK_LOG_ERROR(tmp, ...) Toki::Log(LogLevel::Error, tmp __VA_OPT__(,)__VA_ARGS__)
#define TK_LOG_FATAL(tmp, ...) Toki::Log(LogLevel::Fatal, tmp __VA_OPT__(,)__VA_ARGS__)

#ifdef DEBUG
#define TK_LOG_DEBUG(tmp, ...) Toki::Log(LogLevel::Debug, tmp __VA_OPT__(,)__VA_ARGS__)
#define TK_LOG_TRACE(tmp, ...) Toki::Log(LogLevel::Trace, tmp __VA_OPT__(,)__VA_ARGS__)
#endif