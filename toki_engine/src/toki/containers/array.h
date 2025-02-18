#pragma once

#include "core/base.h"
#include "core/globals.h"

namespace toki {

namespace containers {

template <typename T, u64 SIZE>
struct Array {
    Array() {
        ptr = g_allocator.allocate(SIZE);
    }

    ~Array() {
        g_allocator.free(ptr);
    }

    constexpr u64 size() const {
        return SIZE;
    }

private:
    T* ptr;
};

}  // namespace containers

}  // namespace toki
