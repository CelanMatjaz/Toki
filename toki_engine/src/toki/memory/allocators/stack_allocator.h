#pragma once

#include "core/base.h"

namespace toki {

class StackAllocator {
public:
    StackAllocator() = delete;
    explicit StackAllocator(u32 size);
    ~StackAllocator();

public:
    void* allocate(u32 size);

    void* allocate_aligned(u32 size, u32 alignment);

    template <typename T, typename... Args>
    constexpr T* emplace(Args&&... args) {
        void* ptr = allocate_aligned(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    void free_to_offset(u32 offset);
    void clear();

    u32 get_offset() {
        return m_offset;
    }

private:
    void* m_ptr;
    u32 m_offset;
    u32 m_capacity;
};

}  // namespace toki
