#include "stack_allocator.h"

#include <cstddef>

#include "core/assert.h"
#include "core/base.h"
#include "platform.h"

namespace toki {

StackAllocator::StackAllocator(u32 size): m_ptr(platform::allocate(size)), m_offset(0), m_capacity(size) {}

StackAllocator::~StackAllocator() {
    platform::deallocate(m_ptr);
}

void* StackAllocator::allocate(u32 size) {
    TK_ASSERT(m_offset + size <= m_capacity, "Not enough allocated memory in allocator");
    void* ptr = (((byte*) m_ptr) + (m_offset));
    m_offset += size;
    return ptr;
}

void* StackAllocator::allocate_aligned(u32 size, u32 alignment) {
    TK_ASSERT(alignment >= 1 && alignment <= 128, "Alignment out of bounds");
    TK_ASSERT((alignment & (alignment - 1)) == 0, "Alignment isn't a power of 2");

    u32 total_allocation_size = size + alignment;

    uintptr_t ptr = (uintptr_t) allocate(total_allocation_size);
    uintptr_t mask = (alignment - 1);
    uintptr_t misalignment = (ptr & mask);
    ptrdiff_t adjustment = alignment - misalignment;

    uintptr_t aligned = ptr + adjustment;
    return (void*) aligned;
}

void StackAllocator::free_to_offset(u32 offset) {
    m_offset = offset;
}

void StackAllocator::clear() {
    m_offset = 0;
}

}  // namespace toki
