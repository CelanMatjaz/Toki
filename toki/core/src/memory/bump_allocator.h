#pragma once

#include "allocator.h"
#include "../core/macros.h"

namespace toki {

class BumpAllocator {
public:
    BumpAllocator() = delete;

    BumpAllocator(Allocator& allocator, u64 size): size(size), allocator(allocator) {
        buffer = allocator.allocate(size);
    }

    ~BumpAllocator() {
        allocator.free(buffer);
    }

    DELETE_COPY(BumpAllocator);
    DELETE_MOVE(BumpAllocator);

    template <typename T>
    inline T* allocate(u64 allocation_size) {
        return reinterpret_cast<T*>(allocate(allocation_size));
    }

    void* allocate(u64 allocation_size) {
        TK_ASSERT(offset + allocation_size <= size, "Allocation would overflow bump allocator buffer");
        void* return_ptr = reinterpret_cast<void*>(ptrdiff(buffer) + offset);
        offset += allocation_size;
        return return_ptr;
    }

    void free(void*) {}  // NOOP for concept to work

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
