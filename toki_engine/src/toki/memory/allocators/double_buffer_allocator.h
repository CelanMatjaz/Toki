#pragma once

#include "memory/allocators/stack_allocator.h"

namespace toki {

class DoubleBufferAllocator {
public:
    DoubleBufferAllocator() = delete;
    explicit DoubleBufferAllocator(u64 size);

    void swap();

    StackAllocator* operator->() {
        return &m_allocators[m_index];
    }

private:
    StackAllocator m_allocators[2];
    u32 m_index{};
};

}  // namespace toki
