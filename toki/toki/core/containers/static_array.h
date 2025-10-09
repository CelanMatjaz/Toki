#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/memory/memory.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T, u64 N, CIsAllocator AllocatorType = DefaultAllocator>
class StaticArray {
public:
	constexpr StaticArray() {
		memset(m_data, N * sizeof(T), 0);
	}

	constexpr StaticArray(T&& default_value): StaticArray() {
		for (u64 i = 0; i < N; i++) {
			m_data[i] = default_value;
		}
	}

	constexpr StaticArray(StaticArray&& other) {
		for (u64 i = 0; i < N; i++) {
			m_data[i] = toki::move(other.m_data[i]);
		}
	}

	constexpr StaticArray(const StaticArray& other) {
		toki::memcpy(m_data, other.m_data, N * sizeof(T));
	}

	constexpr StaticArray& operator=(const StaticArray& other) {
		if (&other != this) {
			toki::memcpy(m_data, other.m_data, N * sizeof(T));
		}

		return *this;
	}

	constexpr StaticArray& operator=(StaticArray&& other) {
		if (this != &other) {
			for (u64 i = 0; i < N; i++) {
				m_data[i] = toki::move(other.m_data[i]);
			}
		}
		return *this;
	}

	constexpr ~StaticArray() {}

	constexpr const T& operator[](u64 index) const {
		return m_data[index];
	}

	constexpr T& operator[](u64 index) {
		return m_data[index];
	}

	constexpr const T* data() const {
		return m_data;
	}

	constexpr T* data() {
		return m_data;
	}

	constexpr u64 size() const {
		return N;
	}

	constexpr T& last() const {
		return m_data[N - 1];
	}

	constexpr operator T*() const {
		return m_data;
	}

	constexpr operator const T*() const {
		return m_data;
	}

private:
	T m_data[N];
};

}  // namespace toki
