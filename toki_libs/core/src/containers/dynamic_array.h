#pragma once

#include "../core/core.h"
#include "../memory/allocators/allocator.h"

namespace toki {

// DynamicArray
//
// Class is used for dynamic allocations with different types
// of allocators that match the AllocatorConcept concept.
//
// Memory allocated through an allocator with an instance of
// DynamicArray MUST be manually freed with the same allocator its
// buffer was allocated, if needed. This will most likely not be
// needed if the instance was created with a short-lived bump allocator
// allocation.

template <typename T>
class DynamicArray {
public:
    DynamicArray() = delete;
    ~DynamicArray() = default;

    DynamicArray(AllocatorConcept auto& allocator, u64 size): _size(size) {
        _ptr = reinterpret_cast<T*>(allocator.allocate(size * sizeof(T)));
    }

    DynamicArray(AllocatorConcept auto& allocator, u64 size, T&& default_value): DynamicArray(allocator, size) {
        for (u32 i = 0; i < size; i++) {
            _ptr[i] = default_value;
        }
    }

    DELETE_COPY(DynamicArray);

    DynamicArray(DynamicArray&& other) {
        swap(other);
    }

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
        if (new_size > _size) {
            T* old_ptr = _ptr;
            _ptr = reinterpret_cast<T*>(allocator.allocate(new_size * sizeof(T)));

            for (u32 i = 0; i < _size; i++) {
                _ptr[i] = static_cast<T&&>(old_ptr[i]);
            }

            if (old_ptr != nullptr) {
                allocator.free(old_ptr);
            }
        }
        _size = new_size;
    }

    inline void shrink_to_count(u32 new_size) {
        TK_ASSERT(new_size <= _size, "New size cannot be larger than old size when shrinking");
        _size = new_size;
    }

    inline T& operator[](u64 index) const {
        return _ptr[index];
    }

    inline T* data() const {
        return _ptr;
    }

    inline const u64& get_capacity() const {
        return _size;
    }

    inline const T* pointer() const {
        return _ptr;
    }

    inline void swap(DynamicArray&& other) {
        _ptr = other._ptr;
        _size = other._size;
        other._ptr = nullptr;
        other._size = 0;
    }

private:
    T* _ptr = nullptr;
    u64 _size = 0;
};

}  // namespace toki
