#pragma once

#include "../core/core.h"
#include "../memory/allocators/allocator.h"

namespace toki {

// DynamicArray
//
// Class is used for dynamic allocations with different types
// of allocators that match the AllocatorConcept concept.

template <typename T, typename A = Allocator>
    requires AllocatorConcept<A>
class DynamicArray {
public:
    DynamicArray() = delete;

    DynamicArray(A& allocator, u64 size): mAllocator(allocator), mSize(size) {
        mPtr = reinterpret_cast<T*>(allocator.allocate(size * sizeof(T)));
    }

    DynamicArray(A& allocator, u64 size, T&& default_value): DynamicArray(allocator, size) {
        for (u32 i = 0; i < size; i++) {
            mPtr[i] = default_value;
        }
    }

    DynamicArray(DynamicArray&& other) {
        swap(other);
    }

    ~DynamicArray() {
        mAllocator.free(mPtr);
    }

    DELETE_COPY(DynamicArray);

    DynamicArray& operator=(DynamicArray&& other) {
        if (*this != other) {
            swap(other);
        }
        return *this;
    }

    // Function assumes that the allocator is the same
    // allocator used to allocate previous buffer.
    // This function will NOT resize/reallocate a new
    // buffer if new_size is less than _size.
    void resize(u32 new_size, AllocatorConcept auto& allocator) {
        if (new_size > mSize) {
            T* old_ptr = mPtr;
            mPtr = reinterpret_cast<T*>(allocator.allocate(new_size * sizeof(T)));

            for (u32 i = 0; i < mSize; i++) {
                mPtr[i] = static_cast<T&&>(old_ptr[i]);
            }

            if (old_ptr != nullptr) {
                allocator.free(old_ptr);
            }
        }
        mSize = new_size;
    }

    inline void shrink_to_count(u32 new_size) {
        TK_ASSERT(new_size <= mSize, "New size cannot be larger than old size when shrinking");
        mSize = new_size;
    }

    inline T& operator[](u64 index) const {
        return mPtr[index];
    }

    inline T* data() const {
        return mPtr;
    }

    inline const u64& get_capacity() const {
        return mSize;
    }

    inline const T* pointer() const {
        return mPtr;
    }

    inline u64 size() const {
        return mSize;
    }

    inline void swap(DynamicArray&& other) {
        mPtr = other.mPtr;
        mSize = other.mSize;
        other.mPtr = nullptr;
        other.mSize = 0;
    }

private:
    A& mAllocator;
    T* mPtr = nullptr;
    u64 mSize = 0;
};

}  // namespace toki
