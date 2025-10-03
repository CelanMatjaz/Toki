#include "toki/core/memory/allocator.h"

#include <toki/core/common/assert.h>
#include <toki/core/memory/memory.h>
#include <toki/platform/platform.h>

#include "toki/core/utils/memory.h"
#include "toki/platform/syscalls.h"

namespace toki {

Allocator::Allocator(u64 size, u64 max_free_list_entries): m_size(size) {
	m_buffer = platform::allocate(m_size);
	m_firstFreePtr = reinterpret_cast<MemorySection*>(m_buffer);
	*m_firstFreePtr = {};
}

Allocator::~Allocator() {
	if (m_buffer != nullptr) {
		platform::free(m_buffer);
	}
}

void* Allocator::allocate(u64 size) {
	TK_ASSERT(m_size > 0);
	MemorySection* next_free_block = m_firstFreePtr;

	MemorySection* prev_free_block = next_free_block;
	while (next_free_block->next != nullptr && next_free_block->size < size) {
		prev_free_block = next_free_block;
		next_free_block = next_free_block->next;
	}

	// Last free block
	if (next_free_block->next == nullptr) {
		// Round up block to 16 bytes
		u64ptr next_block = reinterpret_cast<u64ptr>(next_free_block + 1) + size + 15 & static_cast<u64ptr>(~15);
		u32 asd = reinterpret_cast<toki::byte*>(next_block) - reinterpret_cast<toki::byte*>(next_free_block);
		next_free_block->size = size;
		next_free_block->next = reinterpret_cast<MemorySection*>(next_block);
		*next_free_block->next = {};
	} else {
		prev_free_block->next = next_free_block->next;
	}
	m_firstFreePtr = next_free_block->next;

	return reinterpret_cast<void*>(next_free_block + 1);
}

void* Allocator::allocate_aligned(u64 size, u64 alignment) {
	TK_ASSERT((alignment & (alignment - 1)) == 0);

	u64 total_size = size + alignment;
	u64ptr raw_address = reinterpret_cast<u64ptr>(allocate(total_size));

	u64 mask = alignment - 1;
	u64ptr misalignment = raw_address & mask;
	u64ptr adjustment = alignment - misalignment;

	u64ptr aligned_address = raw_address + adjustment;

	return reinterpret_cast<void*>(aligned_address);
}

void* Allocator::reallocate(void* old, u64 size) {
	if (old == nullptr) {
		return allocate(size);
	}

	void* new_ptr = allocate(size);
	toki::memcpy(old, new_ptr, size);
	free(old);
	return new_ptr;
}

void Allocator::free(void* ptr) {
	MemorySection* block = reinterpret_cast<MemorySection*>(ptr) - 1;
	MemorySection* free_block = m_firstFreePtr;

	while ((reinterpret_cast<u64ptr>(free_block + 1) + free_block->size) < reinterpret_cast<u64ptr>(block)) {
		if (free_block->next == nullptr) {
			break;
		}
		free_block = free_block->next;
	}
	block->next = free_block->next;
	free_block->next = block;
}

void Allocator::free_aligned(void* ptr) {
	toki::byte* aligned = reinterpret_cast<toki::byte*>(ptr);
	u64ptr aligned_address = reinterpret_cast<u64ptr>(ptr);
	u64ptr adjustment = static_cast<u64ptr>(aligned[-1]);
	u64ptr raw_address = aligned_address - adjustment;
	free(reinterpret_cast<void*>(raw_address));
}

}  // namespace toki
