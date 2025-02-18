#pragma once

#include "memory/allocators/bump_allocator.h"

namespace toki {

struct DoubleBumpAllocator {
    DoubleBumpAllocator(u64 size, Allocator& allocator):
        bump_allocators(BumpAllocator(size, allocator), BumpAllocator(size, allocator)) {}

    void swap() {
        current_index = (current_index + 1) % 2;
    }

    BumpAllocator& operator->() {
        return bump_allocators[current_index];
    }

private:
    BumpAllocator bump_allocators[2];
    u8 current_index{};
};

}  // namespace toki
