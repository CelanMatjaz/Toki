#pragma once

#include <format>

#include "core/core.h"
#include "platform.h"

namespace toki {

#define TK_ASSERT(condition, message, ...)                    \
    if (auto c = (condition); c) {                            \
    } else {                                                  \
        TK_LOG_FATAL(                                         \
            "Assertion failed {}:{}\n\t{}, result: "          \
            "{}\n\t{}",                                       \
            __FILE__,                                         \
            __LINE__,                                         \
            #condition,                                       \
            c,                                                \
            std::format(message __VA_OPT__(, ) __VA_ARGS__)); \
        platform::debug_break();                              \
        std::unreachable();                                   \
    }

#define TK_ASSERT_ERROR(error)            \
    if (toki::TkError r = error; error) { \
        TK_ASSERT(false, "Error")         \
    }

}  // namespace toki
