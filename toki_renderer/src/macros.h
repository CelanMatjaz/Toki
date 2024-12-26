#pragma once

#include <toki/core.h>

#define TK_ASSERT_VK_RESULT(result, message, ...)                            \
    {                                                                        \
        VkResult res = result;                                               \
        TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) \
    }
