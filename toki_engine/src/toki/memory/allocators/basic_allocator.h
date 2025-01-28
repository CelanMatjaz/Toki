#pragma once

#include <utility>

#include "core/assert.h"
#include "core/base.h"
#include "platform.h"

namespace toki {

static constexpr u64 INVALID_ALLOCATOR_HANDLE = 0;

class BasicAllocator {
public:
    BasicAllocator();
    BasicAllocator(u32 max_allocations, u32 size): m_maxAllocations(max_allocations), m_size(size) {
        u32 allocation_array_size = max_allocations * sizeof(Allocation);
        u32 total_size = allocation_array_size + size;
        m_allocations = reinterpret_cast<Allocation*>(platform::allocate(total_size));
        m_buffer = reinterpret_cast<void*>(m_allocations + max_allocations);
        m_nextFreeBufferPtr = m_buffer;
    }

    ~BasicAllocator() {
        platform::deallocate(m_buffer);
    }

    u64 allocate(u32 size) {
        u64 allocation_size = size + sizeof(u64);
        TK_ASSERT(
            reinterpret_cast<byte*>(m_nextFreeBufferPtr) + allocation_size < reinterpret_cast<byte*>(m_buffer) + m_size,
            "Generic allocator new allocation size ({}) exceeds total buffer size",

            size);

        Allocation* allocation = &m_allocations[m_allocationCount];
        allocation->buffer_ptr = m_nextFreeBufferPtr;
        allocation->size = allocation_size;
        m_nextFreeBufferPtr = reinterpret_cast<byte*>(m_nextFreeBufferPtr) + allocation_size;

        return m_allocationCount++;
    }

    u64 allocate_aligned(u32 size, u32 alignment) {
        TK_ASSERT(alignment >= 1 && alignment <= 128, "Alignment out of bounds");
        TK_ASSERT((alignment & (alignment - 1)) == 0, "Alignment isn't a power of 2");

        u32 total_allocation_size = size + alignment - 1;

        u64 index = allocate(total_allocation_size);

        uintptr_t ptr = reinterpret_cast<uintptr_t>(m_allocations[index].buffer_ptr);
        uintptr_t mask = (alignment - 1);
        uintptr_t misalignment = (ptr & mask);
        ptrdiff_t adjustment = alignment - misalignment;
        uintptr_t aligned = ptr + adjustment;

        m_allocations[index].buffer_ptr = reinterpret_cast<void*>(aligned);

        return index + 1;
    }

    void free(u64 index) {
        Allocation* allocation = &m_allocations[index];
        // Set most significant bit to 1 to indicate if allocation is free
        allocation->size |= in_use_mask;
    }

    void defragment(u32 block_count, u32 max_size) {
        TK_ASSERT(false, "TODO: reimplement");
    }

    void* get(u64 handle) {
        TK_ASSERT(handle != INVALID_ALLOCATOR_HANDLE, "Cannot get pointer with invalid handle");
        if (handle - 1 < m_maxAllocations && (m_allocations[handle - 1].size & in_use_mask)) {
            return nullptr;
        }
        return m_allocations[handle - 1].buffer_ptr;
    }

    template <typename T>
    T* get(u64 handle) {
        return reinterpret_cast<T*>(get(handle));
    }

private:
    inline static constexpr u64 in_use_mask = static_cast<u64>(1) << 63;

    struct Allocation {
        void* buffer_ptr{};
        u64 size;
    };

    u32 m_maxAllocations{};
    u32 m_size{};
    u32 m_allocationCount{};
    Allocation* m_allocations{};
    void* m_buffer{};
    void* m_nextFreeBufferPtr{};
};

}  // namespace toki

#include "basic_ref.h"
