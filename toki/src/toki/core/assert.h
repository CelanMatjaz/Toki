#pragma once

#include "iostream"

#ifndef DIST
#define TK_ASSERT(valIn, msg)                                                                                                                   \
    {                                                                                                                                           \
        auto val = valIn;                                                                                                                       \
        if (val)                                                                                                                                \
            ;                                                                                                                                   \
        else {                                                                                                                                  \
            std::cout << "Toki assertion ERROR: " << val << " in file " << __FILE__ << ':' << __LINE__ << "\n    " << #valIn << "\n    " << msg \
                      << '\n';                                                                                                                  \
            __debugbreak();                                                                                                                     \
            exit(1);                                                                                                                            \
        }                                                                                                                                       \
    }
#define TK_ASSERT_VK_RESULT(val, msg)                                                                                                               \
    if (val == 0)                                                                                                                                   \
        ;                                                                                                                                           \
    else {                                                                                                                                          \
        std::cout << "Vulkan assert ERROR: VkResult = " << val << " in file " << __FILE__ << ':' << __LINE__ << "\n    " << #val << "\n    " << msg \
                  << '\n';                                                                                                                          \
        __debugbreak();                                                                                                                             \
        exit(1); /* DebugBreak(); */                                                                                                                \
    }
#elif
#define TK_ASSERT(val, msg) val
#define TK_ASSERT_VK_RESULT(val, msg) val
#endif  // !DIST
