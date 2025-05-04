#pragma once

#include "bump_allocator.h"

namespace toki {

class DoubleBumpAllocator {
public:
	DoubleBumpAllocator(u64 size): bump_allocators{ BumpAllocator(size), BumpAllocator(size) } {}

	void swap() {
		current_index = (current_index + 1) % 2;
	}

	BumpAllocator* operator->() {
		return &bump_allocators[current_index];
	}

	BumpAllocator& get() {
		return bump_allocators[current_index];
	}

	BumpAllocator& operator*() {
		return bump_allocators[current_index];
	}

	inline operator BumpAllocator&() {
		return bump_allocators[current_index];
	}

private:
	BumpAllocator bump_allocators[2];
	u8 current_index{};
};

}  // namespace toki
