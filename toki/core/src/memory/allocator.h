#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/platform/memory.h>
#include <toki/core/types.h>

namespace toki {

#define ptrdiff(ptr) reinterpret_cast<u64>(ptr)

inline constexpr u64 MAX_FREELIST_ENTRY_COUNT = 512;

class Allocator {
public:
	Allocator() = delete;

	Allocator(u64 size, u64 max_free_list_entries = MAX_FREELIST_ENTRY_COUNT):
		m_total_free_list_entry_count(max_free_list_entries) {
		m_total_size = size + max_free_list_entries * sizeof(FreeListEntry);
		m_buffer = toki::allocate(m_total_size);
		m_free_list = reinterpret_cast<FreeListEntry*>(m_buffer);
		m_next_free_ptr = m_free_list + max_free_list_entries * sizeof(FreeListEntry);
		m_last_free_entry = m_free_list;
	}

	DELETE_COPY(Allocator);
	Allocator(Allocator&& other) = delete;

	Allocator& operator=(Allocator&& other) {
		if (this != &other) {
			m_total_free_list_entry_count = other.m_total_free_list_entry_count;
			m_total_size = other.m_total_size;
			m_buffer = other.m_buffer;
			m_free_list = other.m_free_list;
			m_next_free_ptr = other.m_next_free_ptr;
			m_last_free_entry = other.m_last_free_entry;

			other.m_buffer = nullptr;
		}

		return *this;
	}

	~Allocator() {
		if (m_buffer != nullptr) {
			toki::deallocate(m_buffer);
		}
	}

	void* allocate(u64 size) {
		if (m_total_free_list_entry_count > 0) {
			FreeListEntry* entry = m_free_list;

			while (entry != nullptr && entry->buffer_ptr != nullptr) {
				u64& entry_size = entry_size_ref(entry);

				// Free entry found
				if (entry_size >= size + sizeof(u64)) {
					u64 offset = size;
					// Prevent very small free entries
					if (size - (entry_size + sizeof(u64)) > 16) {
						void* ptr = entry->buffer_ptr;
						entry->buffer_ptr = offset_ptr(entry->buffer_ptr, offset);
						return reinterpret_cast<u64*>(ptr) + 1;
					} else {
						void* ptr = entry->buffer_ptr;
						entry->buffer_ptr = nullptr;
						return reinterpret_cast<u64*>(ptr) + 1;
					}
				}

				entry = entry->next;
			}
			// No free entry found so bump buffer
			goto ALLOCATE_NEW;
		} else {
ALLOCATE_NEW:
			TK_ASSERT(
				ptrdiff(m_next_free_ptr) + size <= ptrdiff(m_buffer) + m_total_size,
				"Allocation would overflow buffer");

			u64* allocation_start = reinterpret_cast<u64*>(m_next_free_ptr);

			u64 offset = size + sizeof(u64);
			offset += offset % 8;

			// Set total allocation size value in the first 64 bits of allocation
			(*allocation_start) = offset;

			// Offset next_free_pointer by allocation size
			m_next_free_ptr = offset_ptr(m_next_free_ptr, offset);

			return allocation_start + 1;
		}

		return nullptr;
	}

	void free(void* ptr) {
		void* allocation_start = reinterpret_cast<u64*>(ptr) - 1;
		u64 allocation_size = *reinterpret_cast<u64*>(allocation_start);
		void* ptr_after_allocation = ptr_after(allocation_start, allocation_size);

		FreeListEntry* entry = m_free_list;
		FreeListEntry* entry_after = nullptr;
		FreeListEntry* entry_before = nullptr;

		// Find free entries that contain pointers that point directly
		// before and directly after the allocation in the buffer
		while (entry->next) {
			// Try to find entry with buffer directly after allocation
			if (entry->buffer_ptr == ptr_after_allocation) {
				entry_after = entry;
				continue;
			}

			// Try to find entry with buffer directly before allocation
			if (entry->buffer_ptr && ptr_after_entry(entry) == allocation_start) {
				entry_before = entry;
				continue;
			}

			entry = entry->next;
		}

		FreeListEntry* new_entry = nullptr;

		// Append allocation buffer range to entry right before it if found
		if (entry_before) {
			new_entry = entry_before;
			entry_size_ref(entry_before) += allocation_size;
		}

		// Prepend allocation buffer range to entry right after it if found
		if (entry_after) {
			// No entry before found - just prepend buffer
			if (new_entry == nullptr) {
				entry_after->buffer_ptr = allocation_start;
				entry_size_ref(entry_after) += allocation_size;
				new_entry = entry_after;
				return;
			}
			// Entry before found - append entry after to entry before
			else {
				new_entry->next = entry_before->next;
				entry_size_ref(entry_after);
			}
		}

		if (new_entry == nullptr) {
			new_entry = find_first_free_entry(m_free_list);
			new_entry->buffer_ptr = allocation_start;
			new_entry->next = find_first_free_entry(new_entry);
		}
	}

	inline u64 size() {
		return m_total_size;
	}

private:
	struct FreeListEntry {
		void* buffer_ptr;
		FreeListEntry* next;
	};

	inline FreeListEntry* find_first_free_entry(FreeListEntry* start) {
		FreeListEntry* entry = start;
		while (entry->buffer_ptr != nullptr) {
			entry++;
		}
		return entry;
	}

	inline static void* offset_ptr(void* ptr, u64 offset) {
		return reinterpret_cast<char*>(ptr) + offset;
	}

	inline static u64& entry_size_ref(FreeListEntry* entry) {
		return *reinterpret_cast<u64*>(entry->buffer_ptr);
	}

	inline static void* ptr_after_entry(FreeListEntry* entry) {
		return ptr_after(entry->buffer_ptr, entry_size_ref(entry));
	}

	inline static void* ptr_after(void* ptr, u64 size) {
		return reinterpret_cast<char*>(ptr) + size;
	}

	void* m_buffer;
	void* m_next_free_ptr;
	u64 m_total_size{};
	FreeListEntry* m_free_list;
	FreeListEntry* m_last_free_entry;
	u64 m_total_free_list_entry_count{};
};

}  // namespace toki
