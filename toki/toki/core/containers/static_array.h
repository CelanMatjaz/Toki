#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/memory/memory.h>

#include "toki/core/utils/memory.h"

namespace toki {

template <typename T, u32 N, CIsAllocator AllocatorType = DefaultAllocator>
class StaticArray {
public:
	StaticArray(): m_data(reinterpret_cast<T*>(AllocatorType::allocate(N * sizeof(T)))) {
		memset(m_data, N * sizeof(T), 0);
	}

	// template <typename... Args>
	// 	requires(Conjunction<IsSame<T, Args>...>::value && sizeof...(Args) == N)
	// StaticArray(Args&&... args): StaticArray() {
	// 	u32 i = 0;
	// 	((m_data[i++] = toki::forward<Args>(args)), ...);
	// }

	StaticArray(T&& default_value): StaticArray() {
		for (u32 i = 0; i < N; i++) {
			m_data[i] = default_value;
		}
	}

	StaticArray(StaticArray&& other): m_data(other.m_data) {
		other.m_data = nullptr;
	}

	~StaticArray() {
		if (m_data != nullptr) {
			AllocatorType::free(m_data);
		}
	}

	DELETE_COPY(StaticArray);

	StaticArray& operator=(StaticArray&& other) {
		if (this != &other) {
			move(toki::move(other));
		}
		return *this;
	}

	inline T& operator[](u64 index) const {
		return m_data[index];
	}

	constexpr const T* data() const {
		return m_data;
	}

	constexpr T* data() {
		return m_data;
	}

	inline u64 size() const {
		return N;
	}

	inline void move(StaticArray&& other) {
		m_data = other.m_data;
		other.m_data = nullptr;
	}

	inline T& last() const {
		return m_data[N - 1];
	}

	inline operator T*() const {
		return m_data;
	}

	inline operator const T*() const {
		return m_data;
	}

private:
	T* m_data{};
};

}  // namespace toki
