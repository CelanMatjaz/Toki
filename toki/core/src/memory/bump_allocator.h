#pragma once

#include "../core/macros.h"
#include "allocator.h"
#include "memory.h"

namespace toki {

class BumpAllocator {
public:
	BumpAllocator() = delete;

	BumpAllocator(u64 size): size(size) {
		buffer = memory_allocate(size);
	}

	~BumpAllocator() {
		memory_free(buffer);
	}

	DELETE_COPY(BumpAllocator);
	DELETE_MOVE(BumpAllocator);

	template <typename T>
	inline T* allocate(u64 allocation_size) {
		return reinterpret_cast<T*>(allocate(allocation_size));
	}

	void* allocate(u64 allocation_size) {
		TK_ASSERT(offset + allocation_size <= size, "Allocation would overflow bump allocator buffer");
		void* return_ptr = reinterpret_cast<void*>(ptrdiff(buffer) + offset);
		offset += allocation_size;
		return return_ptr;
	}

	template <typename T>
	Span<T> allocate_span(u64 count) {
		return Span<T>{ allocate<T>(sizeof(T) * count), count };
	}

	void free(void*) {}	 // NOOP for concept to work

	void free_to_offset(u64 offset) {
		this->offset = offset;
	}

	void clear() {
		offset = 0;
	}

private:
	u64 size;
	u64 offset{};
	void* buffer{};
};

}  // namespace toki
