#pragma once

#include "core/assert.h"
#include "memory/allocators/allocator.h"

namespace toki {

struct BumpAllocator {
    BumpAllocator(u64 size, Allocator& allocator): size(size), allocator(allocator) {
        buffer = allocator.allocate(size);
    }

    ~BumpAllocator() {
        allocator.free(buffer);
    }

    void* bump(u64 allocation_size) {
        TK_ASSERT(offset + allocation_size <= size, "Allocation would overflow bump allocator buffer");
        void* return_ptr = reinterpret_cast<void*>(ptrdiff(buffer) + offset);
        offset += allocation_size;
        return return_ptr;
    }

    void free_to_offset(u64 offset) {
        this->offset = offset;
    }

    void clear() {
        offset = 0;
    }

private:
    u64 size;
    Allocator& allocator;
    u64 offset{};
    void* buffer{};
};

}  // namespace toki
