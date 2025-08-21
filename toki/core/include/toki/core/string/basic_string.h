#pragma once

#include <toki/core/common.h>
#include <toki/core/common/macros.h>
#include <toki/core/memory/memory.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T>
class BasicString {
public:
	constexpr BasicString(): m_size(0), m_ptr(nullptr) {}

	constexpr ~BasicString() {
		if (m_ptr != nullptr) {
			toki::memory_free(m_ptr);
		}
	}

	constexpr BasicString(const T* str):
		m_size(toki::strlen(str)),
		m_ptr(static_cast<T*>(toki::memory_allocate_array<T>(sizeof(str)))) {
		toki::memcpy(str, m_ptr, m_size);
	}

	constexpr BasicString(const T* str, u32 size):
		m_size(size),
		m_ptr(static_cast<T*>(toki::memory_allocate_array<T>(sizeof(str)))) {
		toki::memcpy(str, m_ptr, m_size);
	}

	constexpr BasicString(T* str, u32 size):
		m_size(size),
		m_ptr(static_cast<T*>(toki::memory_allocate_array<T>(m_size))) {
		toki::memcpy(str, m_ptr, m_size);
	}

	constexpr BasicString(u32 size, T ch = 0):
		m_size(size),
		m_ptr(static_cast<T*>(toki::memory_allocate_array<T>(size))) {
		toki::memset(m_ptr, size, ch);
	}

	constexpr BasicString(const BasicString& other):
		m_size(other.m_size),
		m_ptr(static_cast<T*>(toki::memory_allocate_array<T>(other.m_size))) {
		toki::memcpy(other.m_ptr, m_ptr, m_size);
	}

	constexpr BasicString& operator=(const BasicString& other) {
		if (other == this) {
			return this;
		}

		_copy(other);
		return this;
	}

	constexpr BasicString(BasicString&& other): m_size(other.m_size), m_ptr(other.m_ptr) {
		other.m_size = 0;
		other.m_ptr = nullptr;
	}

	constexpr BasicString& operator=(BasicString&& other) {
		if (&other != this) {
			_swap(other);
		}

		return *this;
	}

	constexpr void resize(u32 new_size) {
		if (m_ptr) {
			m_ptr = toki::memory_reallocate_array<T>(m_ptr, m_size = new_size);
		} else {
			m_ptr = toki::memory_allocate_array<T>(m_size = new_size);
		}
	}

	constexpr u32 size() const {
		return m_size;
	}

	constexpr T* data() const {
		return m_ptr;
	}

	T& operator[](u32 pos) {
		return m_ptr[pos];
	}

	operator T*() {
		return m_ptr;
	}

	operator void*() {
		return m_ptr;
	}

private:
	constexpr void _copy(const BasicString& other) {
		m_ptr = memory_allocate_array<T>(m_size = other.m_size);
		memcpy(other.m_ptr, m_ptr, m_size);
	}

	constexpr void _swap(BasicString&& other) {
		toki::swap(m_size, other.m_size);
		toki::swap(m_ptr, other.m_ptr);
	}

private:
	u32 m_size = 0;
	T* m_ptr = nullptr;
};

using String = BasicString<i8>;
using WideString = BasicString<i32>;

}  // namespace toki
