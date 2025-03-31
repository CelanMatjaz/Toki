#pragma once

#include "../../core/types.h"
#include "../../memory/allocators/allocator.h"

namespace toki {

class BumpAllocator {
public:
    BumpAllocator() = delete;

    BumpAllocator(Allocator& allocator, u64 size): mSize(size), mAllocator(allocator) {
        mBuffer = allocator.allocate(size);
    }

    ~BumpAllocator() {
        mAllocator.free(mBuffer);
    }

    DELETE_COPY(BumpAllocator);
    DELETE_MOVE(BumpAllocator);

    template <typename T>
    inline T* allocate(u64 allocation_size) {
        return reinterpret_cast<T*>(allocate(allocation_size));
    }

    void* allocate(u64 allocation_size) {
        TK_ASSERT(mOffset + allocation_size <= mSize, "Allocation would overflow bump allocator buffer");
        void* return_ptr = reinterpret_cast<void*>(ptrdiff(mBuffer) + mOffset);
        mOffset += allocation_size;
        return return_ptr;
    }

    void free(void*) {}  // NOOP for concept to work

    void free_to_offset(u64 offset) {
        this->mOffset = offset;
    }

    void clear() {
        mOffset = 0;
    }

private:
    Allocator& mAllocator;
    u64 mSize{};
    u64 mOffset{};
    void* mBuffer{};
};

}  // namespace toki
