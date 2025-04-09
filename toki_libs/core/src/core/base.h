#pragma once

#include "types.h"

namespace toki {

constexpr u64 KB(u64 value) {
    return 1024 * value;
}

constexpr u64 MB(u64 value) {
    return 1024 * KB(value);
}

constexpr u64 GB(u64 value) {
    return 1024 * MB(value);
}

constexpr u64 BIT(u64 value) {
    return static_cast<u64>(1) << value;
}

}  // namespace toki
