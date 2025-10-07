#include "toki/core/memory/allocator.h"

#include <toki/core/common/assert.h>
#include <toki/core/memory/memory.h>
#include <toki/platform/platform.h>

namespace toki {

Allocator::Allocator(u64 size): m_size(size) {
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

	if (size + sizeof(MemorySection) > m_size ||
		reinterpret_cast<u64ptr>(m_firstFreePtr) >= reinterpret_cast<u64ptr>(m_buffer) + m_size) {
		return nullptr;
	}

	// First free block was never set and is valid to overwrite
	if (m_firstFreePtr->size == 0) {
		m_firstFreePtr->size = size;
		void* ptr = m_firstFreePtr + 1;
		m_firstFreePtr = reinterpret_cast<MemorySection*>(reinterpret_cast<u64ptr>(m_firstFreePtr + 1) + size);
		return ptr;
	}

	MemorySection* next_free_block = m_firstFreePtr;
	MemorySection* previous_free_block = next_free_block;

	// Find first block with required size
	while (next_free_block->size < size && next_free_block->next != nullptr) {
		previous_free_block = next_free_block;
		next_free_block = next_free_block->next;
	}

	// First free block has enough space
	if (next_free_block == m_firstFreePtr) {
		if (m_firstFreePtr->next == nullptr) {
			m_firstFreePtr->next =
				reinterpret_cast<MemorySection*>(reinterpret_cast<u64ptr>(m_firstFreePtr + 1) + size);
		}

		m_firstFreePtr = m_firstFreePtr->next;
		next_free_block->next = nullptr;

		return reinterpret_cast<void*>(next_free_block + 1);
	}

	// No block with required space exists
	if (next_free_block->next == nullptr) {
		next_free_block->size = size;

		previous_free_block->next =
			reinterpret_cast<MemorySection*>(reinterpret_cast<u64ptr>(next_free_block + 1) + size);
		previous_free_block->next = {};

		next_free_block->next = nullptr;
		return next_free_block + 1;
	}

	// Found a block between first and last free block
	previous_free_block->next = next_free_block->next;
	next_free_block->next = nullptr;
	return next_free_block + 1;
}

void* Allocator::allocate_aligned(u64 size, u64 alignment) {
	TK_ASSERT((alignment & (alignment - 1)) == 0);

	u64 total_size = size + alignment;
	u64ptr raw_address = reinterpret_cast<u64ptr>(allocate(total_size));

	u64 mask = alignment - 1;
	u64ptr misalignment = raw_address & mask;
	u64ptr adjustment = alignment - misalignment;

	u64ptr aligned_address = raw_address + adjustment;
	(reinterpret_cast<byte*>(aligned_address))[-1] = adjustment;

	return reinterpret_cast<void*>(aligned_address);
}

void* Allocator::reallocate(void* old, u64 size) {
	if (old == nullptr) {
		return allocate(size);
	}

	void* new_ptr = allocate(size);
	toki::memcpy(new_ptr, old, size);
	free(old);
	return new_ptr;
}

void Allocator::free(void* ptr) {
	MemorySection* block = reinterpret_cast<MemorySection*>(ptr) - 1;

	if (block < m_firstFreePtr) {
		block->next = m_firstFreePtr;
		m_firstFreePtr = block;
		return;
	}

	MemorySection* free_block = m_firstFreePtr;

	// Find first free block before the block to deallocate
	while (free_block->next != nullptr &&
		   reinterpret_cast<u64ptr>(free_block) + free_block->size < reinterpret_cast<u64ptr>(block)) {
		free_block = free_block->next;
	}

	// The block to deallocate is last in line free block
	if (free_block->next == nullptr) {
		block->next = nullptr;
		free_block->next = block;
		return;
	}

	// The block to deallocate is before last free block
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
