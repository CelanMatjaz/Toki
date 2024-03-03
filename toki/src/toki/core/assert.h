#pragma once

#include <print>
#include <stacktrace>

#ifdef DIST

#define TK_ASSERT(val, msg) val
#define TK_ASSERT_VK_RESULT(val, msg) val

#else

#define TK_ASSERT(valIn, msg)                                                                                   \
    if (auto val = valIn; val == 0)                                                                             \
        ;                                                                                                       \
    else {                                                                                                      \
        std::println("Toki assertion ERROR: {} in file {}:{}\n\t{}\n{}", val, __FILE__, __LINE__, #valIn, msg); \
    }

#define TK_ASSERT_VK_RESULT(valIn, msg)                                                                                    \
    if (int32_t val = (int32_t) valIn; val == 0)                                                                           \
        ;                                                                                                                  \
    else {                                                                                                                 \
        std::println("Toki assertion ERROR: VkResult = {} in file {}:{}\n\t{}\n{}", val, __FILE__, __LINE__, #valIn, msg); \
    }

#endif