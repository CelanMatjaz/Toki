#pragma once

#include "core/base.h"
#include "core/globals.h"
#include "core/macros.h"

namespace toki {

namespace containers {

template <typename T>
struct DynamicArray {
    DynamicArray() {}

    DynamicArray(u64 size): size(size) {
        ptr = reinterpret_cast<T*>(g_allocator.allocate(size));
    }

    DynamicArray(u64 size, T&& default_value): DynamicArray(size) {
        for (u32 i = 0; i < size; i++) {
            ptr[i] = default_value;
        }
    }

    ~DynamicArray() {
        g_allocator.free(ptr);
    }

    DELETE_COPY(DynamicArray);
    DELETE_MOVE(DynamicArray);

    void resize(u32 new_size) {
        if (new_size > size) {
            T* old_ptr = ptr;
            ptr = reinterpret_cast<T*>(g_allocator.allocate(new_size));

            for (u32 i = 0; i < size; i++) {
                ptr[i] = static_cast<T&&>(old_ptr[i]);
            }

            size = new_size;
            if (old_ptr != nullptr) {
                g_allocator.free(old_ptr);
            }
        }
    }

    T& operator[](u64 index) const {
        return ptr[index];
    }

    T* data() const {
        return ptr;
    }

    inline const u64& get_capacity() const {
        return size;
    }

private:
    T* ptr = nullptr;
    u64 size = 0;
};

}  // namespace containers

}  // namespace toki
