#pragma once

#include "core/base.h"

namespace toki {

class StackAllocator {
public:
    StackAllocator() = delete;
    explicit StackAllocator(u64 size);
    ~StackAllocator();

public:
    void* allocate(u64 size);

    void* allocate_aligned(u64 size, u64 alignment);

    template <typename T>
    T* allocate_aligned(u64 count) {
        return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T), alignof(T)));
    }

    template <typename T, typename... Args>
    constexpr T* emplace(Args&&... args) {
        void* ptr = allocate_aligned(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    void free_to_offset(u64 offset);
    void clear();

    u64 get_offset() const {
        return m_offset;
    }

private:
    void* m_ptr;
    u64 m_offset;
    u64 m_capacity;
};

}  // namespace toki
