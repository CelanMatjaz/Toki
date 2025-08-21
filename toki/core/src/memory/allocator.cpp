#include "toki/core/memory/allocator.h"

#include <toki/core/memory/memory.h>
#include <toki/core/platform/memory.h>

namespace toki {

Allocator::Allocator(u64 size, u64 max_free_list_entries): m_size(size) {
	m_buffer = toki::allocate(m_size);
	m_firstFreePtr = reinterpret_cast<MemorySection*>(m_buffer);
	*m_firstFreePtr = {};
}

Allocator::~Allocator() {
	if (m_buffer != nullptr) {
		toki::memory_free(m_buffer);
	}
}

void* Allocator::allocate(u64 size) {
	MemorySection* next_free_block = m_firstFreePtr;

	MemorySection* prev_free_block = nullptr;
	while (next_free_block->next != nullptr && next_free_block->size < size) {
		prev_free_block = next_free_block;
		next_free_block = next_free_block->next;
	}

	// Last free block
	if (next_free_block->next == nullptr) {
		// Round up block to 16 bytes
		void* next_block = reinterpret_cast<byte*>(next_free_block + 1) + size + (size & ~16);
		u32 asd = reinterpret_cast<toki::byte*>(next_block) - reinterpret_cast<toki::byte*>(next_free_block);
		next_free_block->size = size;
		next_free_block->next = reinterpret_cast<MemorySection*>(next_block);
		*next_free_block->next = {};
		m_firstFreePtr = next_free_block->next;
	} else {
		prev_free_block->next = next_free_block->next;
	}

	return reinterpret_cast<void*>(next_free_block + 1);
}

void Allocator::free(void* ptr) {
	MemorySection* block = reinterpret_cast<MemorySection*>(ptr) - 1;

	MemorySection* free_block = m_firstFreePtr;
	while ((reinterpret_cast<byte*>(free_block + 1) + free_block->size) < reinterpret_cast<byte*>(block)) {
		free_block = free_block->next;
	}
	block->next = free_block->next;
	free_block->next = block;
}

}  // namespace toki
