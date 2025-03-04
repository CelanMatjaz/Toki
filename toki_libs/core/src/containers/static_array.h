#pragma once

#include "../memory/allocators/allocator.h"

namespace toki {

// StaticArray
//
// Class is used for static allocations with different types
// of allocators that match the AllocatorConcept concept.
//
//
//
// Memory allocated through an allocator with an instance of
// StaticArray MUST be manually freed with the same allocator its
// buffer was allocated, if needed. This will most likely not be
// needed if the instance was created with a short-lived bump allocator
// allocation.

template <typename T, u64 SIZE>
class StaticArray {
public:
    StaticArray() = delete;
    ~StaticArray() = default;

    StaticArray(AllocatorConcept auto& allocator) {
        _ptr = reinterpret_cast<T*>(allocator.allocate(SIZE * sizeof(T)));
    }

    constexpr u64 size() const {
        return SIZE;
    }

    inline const T* pointer() const {
        return _ptr;
    }

    inline T& operator[](u64 index) const {
        return at(index);
    }

    inline T& at(u64 index) const {
        TK_ASSERT(index < SIZE, "Provided index is out of array bounds");
        return _ptr[index];
    }

private:
    T* _ptr = nullptr;
};

}  // namespace toki
