#pragma once

#include "../../core/concepts.h"

namespace toki {

template <typename T>
concept SyscallArgumentConcept = IsConvertibleConcept<T, i64>;

template <typename... Args>
    requires(SyscallArgumentConcept<Args> && ...) && (sizeof...(Args) <= 6)
[[nodiscard]] i64 syscall(u64 _syscall, Args... args) {
    i64 casted_args[sizeof...(Args)] = { ((i64) (args))... };

    i64 ret{};

    if constexpr (sizeof...(Args) == 1) {
        asm volatile("syscall" : "=a"(ret) : "a"(_syscall), "D"(static_cast<i64>(casted_args[0])));
    }

    else if constexpr (sizeof...(Args) == 2) {
        asm volatile("syscall"
                     : "=a"(ret)
                     : "a"(_syscall), "D"(static_cast<i64>(casted_args[0])), "S"(static_cast<i64>(casted_args[1])));
    }

    else if constexpr (sizeof...(Args) == 3) {
        asm volatile("syscall"
                     : "=a"(ret)
                     : "a"(_syscall),
                       "D"(static_cast<i64>(casted_args[0])),
                       "S"(static_cast<i64>(casted_args[1])),
                       "d"(static_cast<i64>(casted_args[2])));
    }

    else if constexpr (sizeof...(Args) == 3) {
        register i64 r10 asm("r10") = static_cast<i64>(casted_args[3]);
        register i64 r8 asm("r8") = static_cast<i64>(casted_args[4]);
        register i64 r9 asm("r9") = static_cast<i64>(casted_args[5]);
        asm volatile("syscall"
                     : "=a"(ret)
                     : "a"(_syscall),
                       "D"(static_cast<i64>(casted_args[0])),
                       "S"(static_cast<i64>(casted_args[1])),
                       "d"(static_cast<i64>(casted_args[2])),
                       "r"(r10),
                       "r"(r8),
                       "r"(r9));
    }

    return ret;
}

}  // namespace toki
