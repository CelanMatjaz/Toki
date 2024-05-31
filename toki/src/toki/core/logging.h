#pragma once

#include <chrono>
#include <print>

namespace Toki {

static const std::chrono::time_zone* currentZone = std::chrono::get_tzdb().current_zone();

#define LOG(tag, str, ...)                                                                                                            \
    std::println(                                                                                                                     \
        "[{:%T}] [{}]: {}",                                                                                                           \
        std::chrono::zoned_time{ currentZone, std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()) }, \
        tag,                                                                                                                          \
        std::format(str __VA_OPT__(, ) __VA_ARGS__))

#define LOG_INFO(str, ...) LOG("Info", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(str, ...) LOG("Warn", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(str, ...) LOG("Error", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(str, ...) LOG("Fatal", str __VA_OPT__(, ) __VA_ARGS__)

#define LOG_TAG(tag, str, ...) LOG(tag, str __VA_OPT__(, ) __VA_ARGS__)

#if defined(TK_NDEBUG)
#define LOG_DEBUG(str, ...)
#else
#define LOG_DEBUG(str, ...) LOG("Debug", str __VA_OPT__(, ) __VA_ARGS__)
#endif

}  // namespace Toki
