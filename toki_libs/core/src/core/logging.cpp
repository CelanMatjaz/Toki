/* #include "logging.h"

#include "../core/types.h"
#include "core/common.h"
#include "core/concepts.h"

namespace toki {

enum STANDARD_FDS {
    STD_IN,
    STD_OUT,
    STD_ERROR,

    STANDARD_FD_COUNT
};

template <typename T>
static consteval void _check_format_args(T arg) {}

template <typename RequiredType, typename Type, typename... Args>
static consteval void _check_format_args(Type arg, Args... args) {
    if constexpr (!IsSameValue<RequiredType, Type>) {
        static_assert(false, "format arg invalid");
    }

    _check_format_args(args...);
}

static consteval b8 _check_format_string(const char* format_str) {
    return false;
}

  ======== Custom print function ========


template <typename... Args>
constexpr void print(const char* format_str, Args... args) {
    static_assert(_check_format_string(format_str), "invalid format string");
}

template <typename... Args>
constexpr void println(const char* format_str, Args... args) {
    print<Args...>((format_str "\n"), args...);
}

void a() {
    print("awdjoiawdjoaw", 1);
}

}  // namespace toki */
