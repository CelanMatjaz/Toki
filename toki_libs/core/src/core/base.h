#pragma once

#include "types.h"

namespace toki {

/* constexpr u64 INVALID_HANDLE_ID = 0;

struct Handle {
    Handle(): unique_id(INVALID_HANDLE_ID), index(0), data(0) {}
    Handle(u32 index, u32 data = 0): unique_id(time(0)), index(index), data(data) {}

    operator bool() {
        return unique_id != INVALID_HANDLE_ID;
    }

    u64 unique_id;
    u32 index;
    u32 data;
}; */

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
