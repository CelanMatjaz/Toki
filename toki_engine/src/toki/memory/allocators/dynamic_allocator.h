#pragma once

#include <utility>

#include "core/assert.h"
#include "core/base.h"
#include "platform.h"

namespace toki {

#define ptrdiff(ptr) reinterpret_cast<u64>(ptr)

static constexpr u64 MAX_FREELIST_ENTRY_COUNT = 128;

class DynamicAllocator {
public:
    DynamicAllocator() = default;

    DynamicAllocator(u64 size /* , u64 freelist_entry_count = MAX_FREELIST_ENTRY_COUNT */):
        // m_freelistEntryCount(freelist_entry_count),
        m_size(size) {
        u64 total_size = MAX_FREELIST_ENTRY_COUNT * sizeof(FreeListEntry) + size;
        m_freeListPtr = reinterpret_cast<FreeListEntry*>(platform::allocate(total_size));
        m_bufferPtr = m_nextFreeBufferPtr = m_freeListPtr + MAX_FREELIST_ENTRY_COUNT;
        m_freeListPtr->next_free = nullptr;
        m_freeListPtr->size = size;
        m_freeListPtr->ptr = m_bufferPtr;
    }

    ~DynamicAllocator() {
        platform::deallocate(m_freeListPtr);
    }

    void* allocate_aligned(u64 size, u64 alignment) {
        TK_ASSERT(alignment >= 1 && alignment <= 64, "Alignment out of bounds");
        TK_ASSERT((alignment & (alignment - 1)) == 0, "Alignment isn't a power of 2");

        u64 total_allocation_size = size + alignment - 1;

        void* raw_ptr = allocate(total_allocation_size);

        uintptr_t ptr = ptrdiff(raw_ptr);
        uintptr_t mask = (alignment - 1);
        uintptr_t misalignment = (ptr & mask);
        ptrdiff_t adjustment = alignment - misalignment;
        uintptr_t aligned = ptr + adjustment;

        return reinterpret_cast<void*>(aligned);
    }

    template <typename T>
    u64 allocate_aligned() {
        allocate_aligned(sizeof(T), alignof(T));
    }

    // TODO: combine free entries if they're next to each other when calling free
    // This will lower the chance of hitting max free entries
    void free(void* ptr) {
        void* raw_ptr = (reinterpret_cast<u64*>(ptr) - 1);
        u64 allocation_size = *reinterpret_cast<u64*>(raw_ptr);

        FreeListEntry* entry = find_first_unused_free_list_entry();
        entry->size = allocation_size;
        entry->ptr = raw_ptr;

        m_lastFreeListEntry->next_free = entry;
        m_lastFreeListEntry = entry;
    }

private:
    void* allocate(u64 size) {
        FreeListEntry* free_list_entry = m_freeListPtr;
        while (free_list_entry->size < size && free_list_entry) {
            free_list_entry = free_list_entry->next_free;
        }

        u64 total_size = size + sizeof(u64);

        // Free entry size if big enough
        if (free_list_entry->next_free && free_list_entry->next_free->size >= total_size) {
            void* free_ptr = free_list_entry->next_free->ptr;

            // Check for free entry padding (64 bytes) and remove/invalidate it from free list
            // Padding is there to remove possibility of very small free sections existing
            if (free_list_entry->next_free->size < total_size + 64) {
                free_list_entry->next_free->ptr = nullptr;
                free_list_entry->next_free = free_list_entry->next_free->next_free;
            }

            // Split free section into 2 and use latter section as new free section
            else {
                free_ptr = free_list_entry->next_free->ptr;
                free_list_entry->next_free->ptr =
                    reinterpret_cast<void*>(ptrdiff(free_list_entry->next_free->ptr) + total_size);
                free_list_entry->next_free->size -= total_size;
            }

            return reinterpret_cast<u64*>(free_ptr) + 1;
        }
        // Return latest buffer pointer
        else {
            u64 allocation_size = size + sizeof(u64);
            TK_ASSERT(
                m_allocatedSize + allocation_size <= m_size, "New allocation would overflow allocated buffer");
            *reinterpret_cast<u64*>(m_nextFreeBufferPtr) = allocation_size;
            m_allocatedSize += allocation_size;            
            void* return_ptr = reinterpret_cast<u64*>(m_nextFreeBufferPtr) + 1;
            m_nextFreeBufferPtr = reinterpret_cast<byte*>(m_nextFreeBufferPtr) + allocation_size;
            return return_ptr;
        }
    }

    struct FreeListEntry {
        void* ptr;
        u64 size;
        FreeListEntry* next_free;
    };

    FreeListEntry* find_first_unused_free_list_entry() {
        for (u32 i = 0; i < MAX_FREELIST_ENTRY_COUNT; i++) {
            if (m_freeListPtr[i].ptr == nullptr) {
                return &m_freeListPtr[i];
            }
        }

        TK_ASSERT(false, "Max freelist entry count exceeded");
        std::unreachable();
    }

    FreeListEntry* m_freeListPtr;
    FreeListEntry* m_lastFreeListEntry;
    // u64 m_freelistEntryCount;
    void* m_bufferPtr;
    void* m_nextFreeBufferPtr;
    u64 m_size;
    u64 m_allocatedSize{};
};

}  // namespace toki
