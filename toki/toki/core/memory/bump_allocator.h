#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>
#include <toki/core/platform/syscalls.h>
#include <toki/core/types.h>

namespace toki {

class BumpAllocator {
public:
	BumpAllocator() = delete;

	BumpAllocator(u64 size): m_data(toki::allocate(size).value_or({})), m_marker(0) {}

	~BumpAllocator() {
		toki::free(m_data);
	}

	void* allocate(u64 size) {
		m_marker += size;
		return &reinterpret_cast<byte*>(m_data)[m_marker - size];
	}

	void* allocate_aligned(u64 size, u64 alignment) {
		TK_ASSERT((alignment & (alignment - 1)) == 0);

		u64 total_size = size + alignment;
		u64ptr raw_address = reinterpret_cast<u64ptr>(allocate(total_size));

		u64 mask = alignment - 1;
		u64ptr misalignment = raw_address & mask;
		u64ptr adjustment = alignment - misalignment;

		u64ptr aligned_address = raw_address + adjustment;

		return reinterpret_cast<void*>(aligned_address);
	}

	u64 get_marker() const {
		return m_marker;
	}

	void free_to_marker(u64 marker = 0) {
		m_marker = marker;
	}

	void reset() {
		free_to_marker();
	}

private:
	void* m_data{};
	u64 m_marker{};
};

}  // namespace toki
