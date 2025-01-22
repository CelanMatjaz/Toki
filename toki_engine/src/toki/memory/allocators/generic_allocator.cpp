#include "generic_allocator.h"

#include <cstring>

#include "core/assert.h"
#include "core/logging.h"
#include "platform.h"

namespace toki {

GenericAllocator::GenericAllocator() {}

GenericAllocator::GenericAllocator(u32 max_allocations, u32 size): m_maxAllocations(max_allocations), m_size(size) {
    u32 allocation_array_size = max_allocations * sizeof(Allocation);
    u32 total_size = allocation_array_size + size;
    m_buffer = platform::allocate(total_size);
    m_nextFreeAllocation = static_cast<Allocation*>(m_buffer);
    Allocation* buffer_start = static_cast<Allocation*>(m_buffer) + max_allocations;
    m_nextFreeBufferPtr = static_cast<void*>(buffer_start);
}

GenericAllocator::~GenericAllocator() {
    platform::deallocate(m_buffer);
}

void* GenericAllocator::get(const void* ptr) {
    if (ptr == nullptr) {
        return nullptr;
    }
    const Allocation* allocation = static_cast<const Allocation*>(ptr);
    return allocation->buffer_ptr;
}

void* GenericAllocator::allocate(u32 size) {
    TK_ASSERT(
        reinterpret_cast<byte*>(m_nextFreeBufferPtr) + size < reinterpret_cast<byte*>(m_buffer) + m_size,
        "Generic allocator new allocation size ({}) exceeds total buffer size",
        size);
    TK_ASSERT(
        m_nextFreeAllocation < reinterpret_cast<Allocation*>(m_buffer) + m_maxAllocations,
        "Generic allocator max allocations ({}) exceeded",
        m_maxAllocations);

    Allocation* allocation = m_nextFreeAllocation;
    allocation->size = size;
    allocation->next_free = allocation + 1;
    allocation->buffer_ptr = m_nextFreeBufferPtr;
    allocation->alignment = 1;
    allocation->in_use = true;

    m_nextFreeAllocation = allocation + 1;
    m_nextFreeBufferPtr = reinterpret_cast<byte*>(m_nextFreeBufferPtr) + size;

    return allocation;
}

void* GenericAllocator::allocate_aligned(u32 size, u32 alignment) {
    TK_ASSERT(alignment >= 1 && alignment <= 128, "Alignment out of bounds");
    TK_ASSERT((alignment & (alignment - 1)) == 0, "Alignment isn't a power of 2");

    u32 total_allocation_size = size + alignment - 1;

    Allocation* new_allocation = reinterpret_cast<Allocation*>(allocate(total_allocation_size));

    uintptr_t ptr = (uintptr_t) allocate(total_allocation_size);
    uintptr_t mask = (alignment - 1);
    uintptr_t misalignment = (ptr & mask);
    ptrdiff_t adjustment = alignment - misalignment;
    uintptr_t aligned = ptr + adjustment;

    TK_LOG_DEBUG("\n\toriginal {}\n\taligned  {}", new_allocation->buffer_ptr, reinterpret_cast<void*>(aligned));

    new_allocation->buffer_ptr = reinterpret_cast<void*>(aligned);
    new_allocation->alignment = static_cast<u8>(alignment);

    return new_allocation;
}

void GenericAllocator::free(void* ptr) {
    Allocation* allocation = reinterpret_cast<Allocation*>(ptr);
    allocation->in_use = false;

    if (allocation < m_nextFreeAllocation) {
        allocation->next_free = m_nextFreeAllocation;
        m_nextFreeAllocation = allocation;
    }
}

void GenericAllocator::defragment(u32 block_count, u32 max_size) {
    TK_LOG_INFO("Generic allocator - defragmenting {} block(s)", block_count);

    Allocation* current_free_allocation = m_nextFreeAllocation;
    Allocation* next_in_use_allocation = current_free_allocation + 1;
    Allocation* end = reinterpret_cast<Allocation*>(m_buffer) + m_maxAllocations;

    while (!next_in_use_allocation->in_use && next_in_use_allocation < end) {
        next_in_use_allocation = next_in_use_allocation->next_free;
        if (!next_in_use_allocation) {
            return;
        }
    }

    // Ensure to preserve alignment when copying
    void* aligned_ptr;
    if (current_free_allocation->alignment != next_in_use_allocation->alignment) {
        u32 alignment = next_in_use_allocation->alignment;
        uintptr_t ptr = (uintptr_t) current_free_allocation->buffer_ptr;
        uintptr_t mask = (alignment - 1);
        uintptr_t misalignment = (ptr & mask);
        ptrdiff_t adjustment = alignment - misalignment;
        uintptr_t aligned = ptr + adjustment;
        aligned_ptr = reinterpret_cast<void*>(aligned);
    } else {
        aligned_ptr = current_free_allocation->buffer_ptr;
    }

    memcpy(aligned_ptr, next_in_use_allocation->buffer_ptr, next_in_use_allocation->size);

    next_in_use_allocation->buffer_ptr = aligned_ptr;
    next_in_use_allocation->in_use = true;

    *current_free_allocation = {};
}

}  // namespace toki
