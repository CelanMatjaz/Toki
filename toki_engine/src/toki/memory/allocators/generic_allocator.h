#pragma once

#include "core/base.h"

namespace toki {

class GenericAllocator {
public:
    GenericAllocator();
    GenericAllocator(u32 max_allocations, u32 size);
    ~GenericAllocator();

    void* allocate(u32 size);
    void* allocate_aligned(u32 size, u32 alignment);

    void free(void* ptr);

    void* get(const void* ptr);

    void defragment(u32 block_count, u32 max_size);

private:
    struct Allocation {
        void* buffer_ptr{};
        Allocation* next_free{};
        u32 size{};
        u8 alignment{};
        b8 in_use{};
    };

    u32 m_maxAllocations;
    u32 m_size;
    void* m_buffer{};
    Allocation* m_nextFreeAllocation{};
    void* m_nextFreeBufferPtr{};
};

}  // namespace toki
