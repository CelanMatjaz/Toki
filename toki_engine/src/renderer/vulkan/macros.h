#pragma once

#include "core/assert.h"

#define TK_ASSERT_VK_RESULT(result, message, ...)                        \
    {                                                                    \
        VkResult res = result;                                           \
        TK_ASSERT(res == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) \
    }
