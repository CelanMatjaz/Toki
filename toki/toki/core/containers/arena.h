#pragma once

#include "toki/core/common/assert.h"
#include "toki/core/common/common.h"
#include "toki/core/common/optional.h"
#include "toki/core/common/type_traits.h"
#include "toki/core/containers/bitset.h"
#include "toki/core/containers/container_types.h"
#include "toki/core/memory/memory.h"

namespace toki {

template <typename T, u64 N, CIsAllocator AllocatorType = DefaultAllocator>
class Arena {
public:
	Arena(): m_data(reinterpret_cast<T*>(AllocatorType::allocate_aligned(sizeof(T) * (N + 1), alignof(T)))) {}

	T& operator[](const Handle handle) const {
		return at(handle);
	}

	T& at(const Handle handle) const {
		TK_ASSERT((handle.m_value - 1) < N);
		return m_data[handle.m_value - 1];
	}

	b8 exists(const Handle handle) const {
		return m_bits[handle.m_value - 1];
	}

	void clear(Handle handle) {
		m_bits.set(handle.m_value - 1, false);
		if constexpr (CHasDestructor<T>) {
			m_data[handle.m_value - 1]->~T();
		}
	}

	template <typename... Args>
	Handle emplace_at_first(Args&&... args) {
		const Optional<u64> index = m_bits.get_first_with_value(false);
		if (!index.has_value()) {
			construct_at(&m_data[0], forward<Args>(args)...);
			return Handle{};
		}

		m_bits.set(index.value(), true);
		construct_at(&m_data[index.value()], forward<Args>(args)...);
		return Handle{ index.value() + 1 };
	}

	void invalidate(Handle handle) {
		invalidate_at_index(handle.m_value - 1);
	}

	void clear() {
		for (u32 i = 0; i < N; i++) {
			invalidate_at_index(i);
		}
	}

private:
	void invalidate_at_index(u64 index) {
		if constexpr (CHasDestructor<T>) {
			destroy_at(&m_data[index - 1]);
		}
		m_bits.set(index, false);
	}

	T* m_data;
	Bitset<N> m_bits;
};

}  // namespace toki
