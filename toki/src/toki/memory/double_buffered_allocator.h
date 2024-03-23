#pragma once

#include "stack_allocator.h"

namespace Toki {

class DoubleBufferedAllocator {
public:
    DoubleBufferedAllocator() = delete;
    explicit DoubleBufferedAllocator(uint32_t allocationSize, MemoryTag tag);

    void* alloc(uint32_t size);
    void swapBuffers();
    void clearCurrentBuffer();

private:
    bool m_currentBuffer = 0;
    StackAllocator m_buffers[2];
}

}  // namespace Toki
