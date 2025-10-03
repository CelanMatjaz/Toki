#pragma once

#include <toki/core/common/macros.h>
#include <toki/core/types.h>

#include "toki/core/common/type_traits.h"

namespace toki {

constexpr const u64 MAX_FREELIST_ENTRY_COUNT = 512;

class Allocator {
public:
	Allocator() = default;

	Allocator(u64 size, u64 max_free_list_entries = MAX_FREELIST_ENTRY_COUNT);
	~Allocator();

	DELETE_COPY(Allocator);

	Allocator& operator=(Allocator&& other) {
		if (&other == this) {
			return *this;
		}

		m_buffer = other.m_buffer;
		m_size = other.m_size;
		m_firstFreePtr = other.m_firstFreePtr;
		other.m_buffer = nullptr;

		return *this;
	}

	void* allocate(u64 size);
	void* allocate_aligned(u64 size, u64 alignment);
	void* reallocate(void* old, u64 size);
	void free(void* ptr);
	void free_aligned(void* ptr);

private:
	struct MemorySection {
		u64 size;
		MemorySection* next;
	};

	void* m_buffer{};
	u64 m_size{};
	MemorySection* m_firstFreePtr{};
};

}  // namespace toki
