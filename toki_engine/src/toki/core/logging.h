#pragma once

#include <chrono>
#include <iostream>
#include <string_view>
#include <utility>

namespace toki {

#if TK_DIST
#define LOG_DEBUG_OFF
#endif

enum LogLevel {
    Info,
    Warn,
    Error,
    Fatal,
    Debug
};

constexpr std::string level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
        case LogLevel::Debug:
            return "DEBUG";
    }

    std::unreachable();
}

template <typename... Args>
constexpr void log(LogLevel level, std::string_view fmt, Args&&... args) {
    static const std::chrono::time_zone* currentZone = std::chrono::get_tzdb().current_zone();
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::zoned_time{ currentZone, std::chrono::time_point_cast<std::chrono::seconds>(now) };
    auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(now));
    std::cout << std::format("[{:%Y-%m-%d} {:%T}] [{}]: ", ymd, time, level_to_string(level))
              << std::vformat(fmt, std::make_format_args(args...)) << '\n';
}

#if defined(LOG_INFO_OFF) || defined(LOGGING_OFF)
#define TK_LOG_INFO(message, ...)
#else
#define TK_LOG_INFO(message, ...) toki::log(toki::LogLevel::Info, message __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_WARN_OFF) || defined(LOGGING_OFF)
#define TK_LOG_WARN(message, ...)
#else
#define TK_LOG_WARN(message, ...) toki::log(toki::LogLevel::Warn, message __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_ERROR_OFF) || defined(LOGGING_OFF)
#define TK_LOG_ERROR(message, ...)
#else
#define TK_LOG_ERROR(message, ...) toki::log(toki::LogLevel::Error, message __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_FATAL_OFF) || defined(LOGGING_OFF)
#define TK_LOG_FATAL(message, ...)
#else
#define TK_LOG_FATAL(message, ...) toki::log(toki::LogLevel::Fatal, message __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_DEBUG_OFF) || defined(LOGGING_OFF)
#define TK_LOG_DEBUG(message, ...)
#else
#define TK_LOG_DEBUG(message, ...) toki::log(toki::LogLevel::Debug, message __VA_OPT__(, ) __VA_ARGS__)
#endif

}  // namespace toki
