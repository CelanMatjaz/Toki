#pragma once

#include "core/assert.h"
#include "core/base.h"
#include "core/macros.h"

namespace toki {

class BasicAllocator {
public:
    BasicAllocator();
    BasicAllocator(u32 max_allocations, u32 size);
    ~BasicAllocator();

    void* allocate(u32 size);
    void* allocate_aligned(u32 size, u32 alignment);
    void free(void* ptr);
    void defragment(u32 block_count, u32 max_size);

    static void* get(const void* ptr);
    template <typename T>
    static T* get(const void* ptr);

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

template <typename T>
class BasicRef {
public:
    BasicRef() = default;
    BasicRef(BasicAllocator* allocator, u64 size = 1): m_allocator(allocator), m_size(size) {
        m_handle = allocator->allocate_aligned(sizeof(T), alignof(T));
    }
    ~BasicRef() {
        m_allocator->free(m_handle);
    };

    void init(BasicAllocator* allocator, u64 size = 1) {
        TK_ASSERT(
            m_handle == nullptr,
            "Calling init on an already initialized BasicRef will cause a memory leak in the allocator");

        m_allocator = allocator;
        m_size = size;
        m_handle = allocator->allocate_aligned(sizeof(T), alignof(T));
    }

    DELETE_COPY(BasicRef);
    DELETE_MOVE(BasicRef);

    T* operator->() const {
        return get();
    }

    T* get() const {
        return BasicAllocator::get<T>(m_handle);
    }

    u64 size() const {
        return m_size;
    }

    T& operator[](const u32 index) const {
        TK_ASSERT(index < size(), "Invalid index");
        return get()[index];
    }

    void reset() {
        m_allocator->free(m_handle);
        m_allocator = nullptr;
        m_handle = nullptr;
        m_size = 0;
    }

private:
    BasicAllocator* m_allocator{};
    void* m_handle{};
    u64 m_size{};
};

}  // namespace toki
