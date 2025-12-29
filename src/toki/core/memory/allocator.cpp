#include "toki/core/memory/allocator.h"

#include <toki/core/common/assert.h>
#include <toki/core/memory/memory.h>
#include <toki/core/platform/syscalls.h>
#include <toki/core/utils/memory.h>

namespace toki {

#define PTR(x)						reinterpret_cast<u64ptr>(x)
#define PTR_TO(x, type)				reinterpret_cast<type*>(reinterpret_cast<void*>(x))
#define BLOCK_AFTER(x)				reinterpret_cast<MemorySection*>(PTR(x + 1) + x->size)
#define ASSERT_CORRECTLY_ALIGNED(x) TK_ASSERT(PTR(x) % alignof(MemorySection) == 0)
#define ASSERT_ALLOCATOR_POINTERS                                                                \
	TK_ASSERT(m_firstFreePtr != nullptr);                                                        \
	TK_ASSERT(m_firstFreePtr >= m_buffer);                                                       \
	TK_ASSERT(m_firstFreePtr->size < m_size);                                                    \
	TK_ASSERT((PTR(m_firstFreePtr->next) + m_firstFreePtr->size) % alignof(MemorySection) == 0); \
	TK_ASSERT(PTR(m_firstFreePtr) != 0x11);

Allocator::Allocator(u64 size): m_size(size) {
	if (size == 0) {
		*this = {};
		return;
	}
	m_buffer		= toki::allocate(size);
	m_firstFreePtr	= reinterpret_cast<MemorySection*>(m_buffer);
	*m_firstFreePtr = {};
}

Allocator::~Allocator() {
	if (m_buffer != nullptr) {
		auto _ = toki::free(m_buffer);
	}
}

void* Allocator::allocate(u64 size) {
	TK_ASSERT(m_size > 0);
	TK_ASSERT(size > 0);
	ASSERT_ALLOCATOR_POINTERS;

	size += (alignof(MemorySection) - (size & (alignof(MemorySection) - 1))) % alignof(MemorySection);

	MemorySection* previous_free_block = nullptr;
	MemorySection* current_free_block  = m_firstFreePtr;

	while (current_free_block->size < size && current_free_block->next != nullptr) {
		previous_free_block = current_free_block;
		current_free_block	= current_free_block->next;
	}

	ASSERT_ALLOCATOR_POINTERS;

	// Last free block
	if (current_free_block->size == 0) {
		ASSERT_ALLOCATOR_POINTERS;
		current_free_block->size = size;
		ASSERT_CORRECTLY_ALIGNED(BLOCK_AFTER(current_free_block));
		if (previous_free_block != nullptr) {
			previous_free_block->next  = BLOCK_AFTER(current_free_block);
			*previous_free_block->next = {};
			ASSERT_CORRECTLY_ALIGNED(previous_free_block->next);
		} else {
			m_firstFreePtr	= BLOCK_AFTER(current_free_block);
			*m_firstFreePtr = {};
			ASSERT_CORRECTLY_ALIGNED(BLOCK_AFTER(m_firstFreePtr));
		}
		ASSERT_CORRECTLY_ALIGNED(m_firstFreePtr);
		current_free_block->next = nullptr;

		if (PTR(current_free_block + 1) + size > PTR(m_buffer) + m_size) {
			return nullptr;
		}

		ASSERT_ALLOCATOR_POINTERS;
		return current_free_block + 1;
	}

	// Free block is big enough to split
	if (static_cast<i64>(current_free_block->size) - size > BLOCK_SPLIT_CUTOFF + sizeof(MemorySection)) {
		u64 original_size		   = current_free_block->size;
		current_free_block->size   = size + size % alignof(MemorySection);
		MemorySection* split_block = BLOCK_AFTER(current_free_block);
		*split_block			   = {};
		split_block->size		   = original_size - (size + sizeof(MemorySection));

		if (previous_free_block != nullptr) {
			previous_free_block->next = current_free_block->next;
		} else {
			split_block->next = m_firstFreePtr->next;
			m_firstFreePtr	  = split_block;
		}

		current_free_block->next = nullptr;

		if (PTR(current_free_block + 1) + size > PTR(m_buffer) + m_size) {
			return nullptr;
		}

		ASSERT_ALLOCATOR_POINTERS;
		return current_free_block + 1;
	}

	// Handle free block from middle of chain
	if (previous_free_block != nullptr) {
		previous_free_block->next = current_free_block->next;
	} else {
		m_firstFreePtr = current_free_block->next;
	}

	// Check if allocated block is inside buffer
	if (PTR(current_free_block + 1) + size > PTR(m_buffer) + m_size) {
		ASSERT_ALLOCATOR_POINTERS;
		return nullptr;
	}

	ASSERT_ALLOCATOR_POINTERS;
	return current_free_block + 1;
}

void* Allocator::allocate_aligned(u64 size, u64 alignment) {
	TK_ASSERT((alignment & (alignment - 1)) == 0);
	ASSERT_ALLOCATOR_POINTERS;

	u64 total_size	   = size + alignment;
	u64ptr raw_address = reinterpret_cast<u64ptr>(allocate(total_size));

	u64 mask			= alignment - 1;
	u64ptr misalignment = raw_address & mask;
	u64ptr adjustment	= alignment - misalignment;
	TK_ASSERT(adjustment > 0);

	u64ptr aligned_address						   = raw_address + adjustment;
	(reinterpret_cast<byte*>(aligned_address))[-1] = static_cast<byte>(adjustment);

	ASSERT_ALLOCATOR_POINTERS;
	return reinterpret_cast<void*>(aligned_address);
}

void* Allocator::reallocate(void* old, u64 size) {
	ASSERT_ALLOCATOR_POINTERS;

	if (old == nullptr) {
		return allocate(size);
	}

	void* new_ptr = allocate(size);
	toki::memcpy(new_ptr, old, size);
	free(old);

	ASSERT_ALLOCATOR_POINTERS;
	return new_ptr;
}

void Allocator::free(void* ptr) {
	if (ptr == nullptr) {
		return;
	}

	MemorySection* block = reinterpret_cast<MemorySection*>(ptr) - 1;

	// Block is before first free ptr
	if (block < m_firstFreePtr) {
		// If current block and first free block
		// are one after another, join them
		if (BLOCK_AFTER(block) == m_firstFreePtr) {
			block->next = m_firstFreePtr->next;
			if (m_firstFreePtr->size == 0) {
				block->size = 0;

			} else {
				block->size += sizeof(MemorySection) + m_firstFreePtr->size;
			}
		} else {
			block->next = m_firstFreePtr;
		}

		m_firstFreePtr = block;
		ASSERT_ALLOCATOR_POINTERS;
		return;
	}

	MemorySection* current_free_block = m_firstFreePtr;
	while (current_free_block < block) {
		current_free_block = current_free_block->next;
	}

	// Join block before block to be freed
	if (BLOCK_AFTER(current_free_block) == block) {
		current_free_block->size += sizeof(MemorySection) + block->size;
	}

	// Join block after block to be freed
	if (BLOCK_AFTER(current_free_block) == current_free_block->next) {
		current_free_block->size += sizeof(MemorySection) + current_free_block->next->size;
		current_free_block->next = current_free_block->next->next;
	}

	ASSERT_ALLOCATOR_POINTERS;
}

void Allocator::free_aligned(void* ptr) {
	ASSERT_ALLOCATOR_POINTERS;
	toki::byte* aligned	   = reinterpret_cast<toki::byte*>(ptr);
	u64ptr aligned_address = reinterpret_cast<u64ptr>(ptr);
	u64ptr adjustment	   = static_cast<u64ptr>(aligned[-1]);
	u64ptr raw_address	   = aligned_address - adjustment;
	free(reinterpret_cast<void*>(raw_address));
	ASSERT_ALLOCATOR_POINTERS;
}

}  // namespace toki
