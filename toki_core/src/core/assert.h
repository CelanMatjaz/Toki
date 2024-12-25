#pragma once

#include "error.h"
#include "logging.h"

namespace toki {

#define TK_ASSERT(condition, message, ...)                    \
    if (auto c = condition; c) {                              \
    } else {                                                  \
        TK_LOG_FATAL(                                         \
            "Assertion failed {}:{}\nAssertion: {}, result: " \
            "{}\n{}",                                         \
            __FILE__,                                         \
            __LINE__,                                         \
            #condition,                                       \
            c,                                                \
            std::format(message __VA_OPT__(, ) __VA_ARGS__)); \
    }

#define TK_ASSERT_ERROR(error)            \
    if (toki::TkError r = error; error) { \
        TK_ASSERT(false, "Error")         \
    }

}  // namespace toki
