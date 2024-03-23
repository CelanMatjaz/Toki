#include "double_buffered_allocator.h"

namespace Toki {

DoubleBufferedAllocator::DoubleBufferedAllocator(uint32_t allocationSize, MemoryTag tag) :
    m_buffers{ StackAllocator(allocationSize, tag), StackAllocator(allocationSize, tag) } {}

void* DoubleBufferedAllocator::alloc(uint32_t size) {
    return m_buffers[m_currentBuffer].alloc(size);
}

void DoubleBufferedAllocator::swapBuffers() {
    m_currentBuffer = !m_currentBuffer;
}

void DoubleBufferedAllocator::clearCurrentBuffer() {
    m_buffers[m_currentBuffer].clear();
}

}  // namespace Toki
