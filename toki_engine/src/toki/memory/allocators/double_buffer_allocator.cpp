#include "double_buffer_allocator.h"

namespace toki {

DoubleBufferAllocator::DoubleBufferAllocator(u32 size): m_allocators{ StackAllocator(size), StackAllocator(size) } {}

void DoubleBufferAllocator::swap() {
    m_index = (m_index + 1) % 2;
}

}  // namespace toki
