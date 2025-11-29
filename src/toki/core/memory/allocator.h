#pragma once

#include <toki/core/common/macros.h>
#include <toki/core/types.h>

namespace toki {

class Allocator {
public:
	Allocator() = default;

	Allocator(u64 size);
	~Allocator();

	DELETE_COPY(Allocator)

	Allocator& operator=(Allocator&& other) {
		if (&other == this) {
			return *this;
		}

		m_buffer	   = other.m_buffer;
		m_size		   = other.m_size;
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

	// If splitting a block, require the 2nd block to
	// be at least `BLOCK_SPLIT_CUTOFF` bytes in size
	constexpr static u64 BLOCK_SPLIT_CUTOFF = 64;

	void* m_buffer{};
	u64 m_size{};
	MemorySection* m_firstFreePtr{};
};

}  // namespace toki
