#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T, u64 N, CIsAllocator AllocatorType = DefaultAllocator>
	requires(N > 0)
class Array {
public:
	constexpr Array() = default;

	template <typename... Args>
		requires(sizeof...(Args) == N && (CIsConvertible<Args, T> && ...))
	constexpr Array(Args&&... args): m_data{ static_cast<T>(toki::forward<Args>(args))... } {}

	constexpr Array(T&& default_value): Array() {
		for (u64 i = 0; i < N; i++) {
			m_data[i] = default_value;
		}
	}

	constexpr Array(const T& default_value): Array() {
		for (u64 i = 0; i < N; i++) {
			m_data[i] = default_value;
		}
	}

	constexpr Array(Array&& other) {
		for (u64 i = 0; i < N; i++) {
			m_data[i] = toki::move(other.m_data[i]);
		}
	}

	constexpr Array(const Array& other) {
		if (*this != other) {
			for (u32 i = 0; i < 16; ++i) {
				m_data[i] = other.m_data[i];
			}
		}
	}

	constexpr Array& operator=(const Array& other) {
		if (&other != this) {
			toki::memcpy(m_data, other.m_data, N * sizeof(T));
		}

		return *this;
	}

	constexpr Array& operator=(Array&& other) {
		if (this != &other) {
			for (u64 i = 0; i < N; i++) {
				m_data[i] = toki::move(other.m_data[i]);
			}
		}
		return *this;
	}

	constexpr ~Array() {}

	template <typename Type, u64 Num>
	friend constexpr bool operator==(const Array<Type, Num>& lhs, const Array<Type, Num>& rhs);

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

	constexpr T& front() const {
		return m_data[0];
	}

	constexpr T& back() const {
		return m_data[N - 1];
	}

	constexpr operator T*() const {
		return m_data;
	}

	constexpr operator const T*() const {
		return m_data;
	}

private:
	T m_data[N]{};
};

template <class T, u64 N>
constexpr bool operator==(const Array<T, N>& lhs, const Array<T, N>& rhs) {
	for (u32 i = 0; i < N; i++) {
		if (lhs.m_data[i] != rhs.m_data[i]) {
			return false;
		}
	}

	return true;
}

}  // namespace toki
