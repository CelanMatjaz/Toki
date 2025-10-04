#pragma once

#include <toki/core/common/common.h>
#include <toki/core/common/macros.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
	requires Disjunction<IsSame<T, char>, IsSame<T, wchar>>::value
class BasicString {
public:
	constexpr BasicString(): m_size(0), m_ptr(nullptr) {}

	constexpr ~BasicString() {
		if (m_ptr != nullptr) {
			AllocatorType::free(m_ptr);
		}
	}

	constexpr BasicString(const T* str):
		m_size(toki::strlen(str)),
		m_ptr(static_cast<T*>(AllocatorType::allocate_aligned(m_size * sizeof(T), alignof(T)))) {
		toki::memcpy(m_ptr, str, m_size * sizeof(T));
	}

	constexpr BasicString(const T* str, u64 size):
		m_size(size),
		m_ptr(static_cast<T*>(AllocatorType::allocate_aligned(m_size * sizeof(T), alignof(T)))) {
		toki::memcpy(m_ptr, str, m_size * sizeof(T));
	}

	constexpr BasicString(T* str, u64 size):
		m_size(size),
		m_ptr(static_cast<T*>(AllocatorType::allocate_aligned(m_size * sizeof(T), alignof(T)))) {
		toki::memcpy(m_ptr, str, m_size);
	}

	constexpr BasicString(u64 size, T ch = 0):
		m_size(size),
		m_ptr(static_cast<T*>(AllocatorType::allocate_aligned(size * sizeof(T), alignof(T)))) {
		toki::memset(m_ptr, size, ch);
	}

	constexpr BasicString(const BasicString& other):
		m_size(other.m_size),
		m_ptr(static_cast<T*>(AllocatorType::allocate_aligned(other.m_size * sizeof(T), alignof(T)))) {
		toki::memcpy(m_ptr, other.m_ptr, m_size);
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

	constexpr void resize(u64 new_size) {
		m_ptr = AllocatorType::reallocate_aligned(m_ptr, new_size * sizeof(T), alignof(T));
	}

	constexpr u64 size() const {
		return m_size;
	}

	constexpr T* data() const {
		return m_ptr;
	}

	T& operator[](u64 pos) {
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
		m_ptr = AllocatorType::allocate_aligned(m_size = (other.m_size * sizeof(T)), alignof(T));
		memcpy(other.m_ptr, m_ptr, other.m_size * sizeof(T));
	}

	constexpr void _swap(BasicString&& other) {
		toki::swap(m_size, other.m_size);
		toki::swap(m_ptr, other.m_ptr);
	}

private:
	u64 m_size = 0;
	T* m_ptr = nullptr;
};

using String = BasicString<char>;
using WideString = BasicString<wchar>;

}  // namespace toki
