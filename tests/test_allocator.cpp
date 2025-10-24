#include "testing.h"
#include "toki/core/memory/allocator.h"
//

#include <toki/core/core.h>

using namespace toki;

#define BUFFER reinterpret_cast<u64ptr>(allocator.m_buffer)
#define OFFSET(x) (reinterpret_cast<u64ptr>(x) - BUFFER)

constexpr const u32 MEMORY_SECTION_SIZE = sizeof(Allocator::MemorySection);

TK_TEST(Allocator, allocates_and_deallocates) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr = allocator.allocate(64);
	TK_TEST_ASSERT(ptr != nullptr);
	allocator.free(ptr);

	return true;
}

TK_TEST(Allocator, allocates_multiple_distinct_blocks) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr1 = allocator.allocate(64);
	void* ptr2 = allocator.allocate(64);
	TK_TEST_ASSERT(ptr1 != ptr2);
	TK_TEST_ASSERT(OFFSET(ptr2) == OFFSET(ptr1) + 64 + MEMORY_SECTION_SIZE);
	allocator.free(ptr1);
	allocator.free(ptr2);

	return true;
}

TK_TEST(Allocator, double_free_works) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr = allocator.allocate(64);
	for (u32 i = 0; i < 100; i++) {
		allocator.free(ptr);
	}

	return true;
}

TK_TEST(Allocator, allocates_correctly_aligned_pointer) {
	toki::Allocator allocator(toki::MB(10));

	for (u32 i = 1; i < 6; i++) {
		u32 alignment = toki::pow(1, i);
		void* ptr = allocator.allocate_aligned(64, alignment);
		TK_TEST_ASSERT(OFFSET(ptr) % alignment == 0);
	}

	return true;
}

TK_TEST(Allocator, should_use_freed_memory_block_when_the_size_is_greater_or_equal_to_allocated_size) {
	// Test smaller size
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate(64);
		allocator.free(ptr1);

		void* ptr2 = allocator.allocate(32);
		TK_TEST_ASSERT(ptr1 == ptr2);
	}

	// Test equal size
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate(64);
		allocator.free(ptr1);

		void* ptr2 = allocator.allocate(64);
		TK_TEST_ASSERT(ptr1 == ptr2);
	}

	// Test greater size
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate(64);
		void* ptr2 = allocator.allocate(64);
		allocator.free(ptr1);

		void* ptr3 = allocator.allocate(65);
		TK_TEST_ASSERT(ptr1 != ptr3);
	}

	// Test smaller size, aligned
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate_aligned(64, 8);
		allocator.free_aligned(ptr1);

		void* ptr2 = allocator.allocate_aligned(32, 8);
		TK_TEST_ASSERT(ptr1 == ptr2);
	}

	// Test equal size, aligned
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate_aligned(64, 8);
		allocator.free_aligned(ptr1);

		void* ptr2 = allocator.allocate_aligned(64, 8);
		TK_TEST_ASSERT(ptr1 == ptr2);
	}

	// Test greater size, aligned
	{
		toki::Allocator allocator(toki::MB(10));
		void* ptr1 = allocator.allocate_aligned(64, 8);
		void* ptr2 = allocator.allocate_aligned(64, 8);
		allocator.free_aligned(ptr1);

		void* ptr3 = allocator.allocate_aligned(65, 8);
		TK_TEST_ASSERT(ptr1 != ptr3);
	}

	return true;
}

TK_TEST(Allocator, returns_nullptr_when_requested_size_is_greater_than_pool) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr = allocator.allocate(toki::MB(11));
	TK_TEST_ASSERT(ptr == nullptr);

	return true;
}

TK_TEST(Allocator, freeing_doesnt_allocate_overlapping_chunks) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr1 = allocator.allocate(16);
	void* ptr2 = allocator.allocate(32);
	void* ptr3 = allocator.allocate(48);

	TK_TEST_ASSERT(ptr1 != nullptr && ptr2 != nullptr && ptr3 != nullptr);

	Allocator::MemorySection* current_free_ptr = allocator.m_firstFreePtr;
	allocator.free(ptr2);
	TK_TEST_ASSERT(ptr2 == allocator.m_firstFreePtr + 1);
	TK_TEST_ASSERT(allocator.m_firstFreePtr->next == current_free_ptr);

	current_free_ptr = allocator.m_firstFreePtr->next;
	void* ptr4 = allocator.allocate(24);
	TK_TEST_ASSERT(allocator.m_firstFreePtr == current_free_ptr);

	TK_TEST_ASSERT(ptr4 != nullptr);
	TK_TEST_ASSERT(ptr4 != ptr1);
	TK_TEST_ASSERT(ptr4 != ptr3);

	allocator.free(ptr4);

	void* ptr5 = allocator.allocate(64);
	TK_TEST_ASSERT(OFFSET(ptr5) == OFFSET(ptr3) + 48 + MEMORY_SECTION_SIZE)

	allocator.free(ptr1);
	allocator.free(ptr3);
	allocator.free(ptr4);
	return true;
}

TK_TEST(Allocator, returns_valid_pointer_if_exact_pool_size_is_requested) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr = allocator.allocate(toki::MB(10) - MEMORY_SECTION_SIZE);
	TK_TEST_ASSERT(ptr != nullptr);

	return true;
}

TK_TEST(Allocator, exhaust_and_reuse_buffer) {
	constexpr u64 N = 8;
	Allocator allocator(N * (64 + MEMORY_SECTION_SIZE));
	void* ptrs[N];

	for (size_t i = 0; i < N; ++i) {
		ptrs[i] = allocator.allocate(64);
		TK_TEST_ASSERT(ptrs[i] != nullptr);
	}

	void* extra = allocator.allocate(64);
	TK_TEST_ASSERT(extra == nullptr);

	allocator.free(ptrs[3]);
	void* new_block = allocator.allocate(64);
	TK_TEST_ASSERT(new_block != nullptr);

	for (size_t i = 0; i < N; ++i) {
		if (ptrs[i])
			allocator.free(ptrs[i]);
	}

	if (new_block)
		allocator.free(new_block);
	return true;
}

TK_TEST(Allocator, should_allocate_and_deallocate_without_problems) {
	toki::Allocator allocator(toki::MB(10));

	void* first_ptr = allocator.allocate(32);
	allocator.free(first_ptr);

	for (int i = 0; i < 1000; ++i) {
		void* ptr = allocator.allocate(32);
		TK_TEST_ASSERT(ptr != nullptr);
		TK_TEST_ASSERT(ptr == first_ptr);
		allocator.free(ptr);
	}

	return true;
}

TK_TEST(Allocator, should_handle_multiple_sized_allocations) {
	toki::Allocator allocator(toki::MB(10));

	void* small = allocator.allocate(8);
	void* medium = allocator.allocate(64);
	void* large = allocator.allocate(256);

	TK_TEST_ASSERT(small && medium && large);

	allocator.free(medium);
	void* medium2 = allocator.allocate(64);
	TK_TEST_ASSERT(medium2 != nullptr);

	allocator.free(small);
	allocator.free(large);
	allocator.free(medium2);

	return true;
}

TK_TEST(Allocator, handles_fragmentation_correctly) {
	toki::Allocator allocator(toki::MB(10));

	void* blocks[10];
	for (int i = 0; i < 10; ++i)
		blocks[i] = allocator.allocate(32 * (i + 1));

	for (int i = 0; i < 10; i += 2)
		allocator.free(blocks[i]);

	void* new_block = allocator.allocate(128);
	TK_TEST_ASSERT(new_block != nullptr);
	allocator.free(new_block);

	for (int i = 1; i < 10; i += 2)
		allocator.free(blocks[i]);
	return true;
}

TK_TEST(Allocator, allocates_contiguous_memory) {
	toki::Allocator allocator(toki::MB(10));
	u64ptr buffer_start = BUFFER;

	constexpr const u32 allocation_count = 16;
	constexpr const u32 allocation_size = 64;
	for (u32 i = 0; i < allocation_count; i++) {
		u64ptr ptr = reinterpret_cast<u64ptr>(allocator.allocate(allocation_size)) - buffer_start;
		u64ptr first_free_ptr = reinterpret_cast<u64ptr>(allocator.m_firstFreePtr) - buffer_start;

		// Test returned pointer offset
		{
			const u32 expected_allocation_offset = (allocation_size + MEMORY_SECTION_SIZE) * i;
			u64ptr expected_offset = MEMORY_SECTION_SIZE + expected_allocation_offset;
			TK_TEST_ASSERT(ptr == expected_offset);
		}

		// Test first free pointer offset
		{
			u64ptr expected_offset = (i + 1) * (MEMORY_SECTION_SIZE + allocation_size);
			TK_TEST_ASSERT(first_free_ptr == expected_offset);
		}
	}

	return true;
}

TK_TEST(Allocator, allocates_same_block_of_memory_if_freed_and_allocation_has_same_size) {
	toki::Allocator allocator(toki::MB(10));
	u64ptr buffer_start = BUFFER;

	constexpr const u32 allocation_count = 16;
	constexpr const u32 allocation_size = 64;
	for (u32 i = 0; i < allocation_count; i++) {
		void* raw_ptr = allocator.allocate(allocation_size);
		u64ptr ptr = reinterpret_cast<u64ptr>(raw_ptr) - buffer_start;
		u64ptr first_free_ptr = reinterpret_cast<u64ptr>(allocator.m_firstFreePtr) - buffer_start;

		// Test returned pointer offset
		{
			u64ptr expected_offset = MEMORY_SECTION_SIZE;
			TK_TEST_ASSERT(ptr == expected_offset);
		}

		// Test first free pointer offset
		{
			u64ptr expected_offset = (MEMORY_SECTION_SIZE + allocation_size);
			TK_TEST_ASSERT(first_free_ptr == expected_offset);
		}

		allocator.free(reinterpret_cast<void*>(raw_ptr));
	}

	return true;
}

TK_TEST(Allocator, random_allocations) {
	toki::Allocator allocator(toki::MB(10));

	u32 allocation_sizes[] = { 14,	675765, 1275, 24346, 33467,	 1235, 47893, 54645, 35342,
							   768, 909085, 1235, 563,	 453645, 232,  769,	  1278 };
	for (u32 i = 0; i < ARRAY_SIZE(allocation_sizes) - 1; i++) {
		void* ptr = allocator.allocate(allocation_sizes[i]);
		allocator.free(ptr);
		ptr = allocator.allocate(allocation_sizes[i + 1]);
		if (i % 3 == 0) {
			allocator.free(ptr);
		}
	}

	return true;
}

TK_TEST(Allocator, split_free_block) {
	toki::Allocator allocator(toki::MB(10));

	void* ptr1 = allocator.allocate(toki::MB(1));
	allocator.free(ptr1);

	void* ptr2 = allocator.allocate(toki::KB(1));
	Allocator::MemorySection* ptr2_block = reinterpret_cast<Allocator::MemorySection*>(ptr2) - 1;

	TK_TEST_ASSERT(
		allocator.m_firstFreePtr ==
		reinterpret_cast<void*>((reinterpret_cast<byte*>(ptr2_block) + MEMORY_SECTION_SIZE + ptr2_block->size)));

	return true;
}
