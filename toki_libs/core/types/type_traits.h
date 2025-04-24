#pragma once

#include "core/concepts.h"
#include "types.h"

namespace toki {

template <typename T>
struct Type {
    static constexpr u64 size = sizeof(T);
    static constexpr T min = IsSignedValue<T> ? static_cast<T>(static_cast<T>(1) << (sizeof(T) * 8 - 1)) : 0;
    static constexpr T max = IsSignedValue<T> ? ~min : ~static_cast<T>(0);
};

}  // namespace toki
