#pragma once

#include <print>

#ifdef TK_DIST

#define TK_ASSERT(val, msg, ...) val
#define TK_ASSERT_VK_RESULT(val, msg, ...) val

#else

#include <source_location>

#define LOG_SOURCE_LOCATION                                                                                                \
    {                                                                                                                      \
        auto location = std::source_location::current();                                                                   \
        std::println("{}:{}:{} '{}'", location.file_name(), location.line(), location.column(), location.function_name()); \
    }

#define TK_ASSERT(valIn, msg, ...)                                                                                                             \
    if (bool val = valIn; val)                                                                                                                 \
        ;                                                                                                                                      \
    else {                                                                                                                                     \
        std::println(                                                                                                                          \
            "Toki assertion ERROR: {} in file {}:{}\n\t{}\n{}", val, __FILE__, __LINE__, #valIn, std::format(msg __VA_OPT__(, ) __VA_ARGS__)); \
        LOG_SOURCE_LOCATION;                                                                                                                   \
        __builtin_trap();                                                                                                                      \
    }

#define TK_ASSERT_VK_RESULT(valIn, msg, ...)                               \
    if (int32_t val = (int32_t) valIn; val == 0)                           \
        ;                                                                  \
    else {                                                                 \
        std::println(                                                      \
            "Toki assertion ERROR: VkResult = {} in file {}:{}\n\t{}\n{}", \
            val,                                                           \
            __FILE__,                                                      \
            __LINE__,                                                      \
            #valIn,                                                        \
            std::format(msg __VA_OPT__(, ) __VA_ARGS__));                  \
        LOG_SOURCE_LOCATION;                                               \
        __builtin_trap();                                                  \
    }

#endif
