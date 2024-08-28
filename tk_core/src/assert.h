#pragma once

#ifdef TK_DIST

#define TK_ASSERT(val, msg, ...) val
#define TK_ASSERT_VK_RESULT(val, msg, ...) val

#else

#include <stacktrace>

#define TK_ASSERT(valIn, msg, ...)                              \
    if (bool val = (valIn); val)                                \
        ;                                                       \
    else {                                                      \
        std::println(                                           \
            std::cerr,                                          \
            "Toki assertion ERROR: {} in file {}:{}\n\t{}\n{}", \
            val,                                                \
            __FILE__,                                           \
            __LINE__,                                           \
            #valIn,                                             \
            std::format(msg __VA_OPT__(, ) __VA_ARGS__));       \
        std::cerr << std::stacktrace::current() << '\n';        \
        __builtin_trap();                                       \
    }

#define TK_ASSERT_VK_RESULT(val, msg, ...) val

#endif
