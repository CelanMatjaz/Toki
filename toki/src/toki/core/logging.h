#pragma once

#include <print>

namespace Toki {

#define LOG(level, str, ...) std::println("[{}]: {}", level, std::format(str __VA_OPT__(, ) __VA_ARGS__))

#define LOG_INFO(str, ...) LOG("INFO", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(str, ...) LOG("WARN", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(str, ...) LOG("ERROR", str __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(str, ...) LOG("INFO", str __VA_OPT__(, ) __VA_ARGS__)

#ifdef TK_NDEBUG
#define LOG_DEBUG(str, ...)
#else
#define LOG_DEBUG(str, ...) LOG("DEBUG", str __VA_OPT__(, ) __VA_ARGS__)
#endif

}  // namespace Toki
