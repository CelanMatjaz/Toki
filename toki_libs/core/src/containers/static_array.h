#pragma once

#include "../memory/allocator.h"

namespace toki {

// StaticArray
//
// Class is used for static allocations with different types
// of allocators that match the AllocatorConcept concept.

template <typename T, u64 SIZE, typename A = Allocator>
    requires AllocatorConcept<A>
class StaticArray {
public:
    StaticArray() = delete;

    StaticArray(A& allocator):
        mAllocator(allocator),
        mPtr(reinterpret_cast<T*>(allocator.allocate(SIZE * sizeof(T)))) {}

    ~StaticArray() {
        if (mPtr != nullptr) {
            mAllocator.free(mPtr);
        }
    }

    StaticArray& operator=(StaticArray&& other) {
        if (this != &other) {
            mAllocator = other.mAllocator;
            mPtr = other.mPtr;

            other.mPtr = nullptr;
        }

        return *this;
    }

    constexpr u64 size() const {
        return SIZE;
    }

    inline T* data() const {
        return mPtr;
    }

    inline T& operator[](u64 index) const {
        return at(index);
    }

    inline T& at(u64 index) const {
        TK_ASSERT(index < SIZE, "Provided index is out of array bounds");
        return mPtr[index];
    }

private:
    A& mAllocator;
    T* mPtr = nullptr;
};

}  // namespace toki
