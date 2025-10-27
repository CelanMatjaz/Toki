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
	static constexpr u32 STACK_SPACE = 24;
	static constexpr u32 STACK_VS_HEAP_CUTOFF = STACK_SPACE / sizeof(T);

public:
	constexpr BasicString(): m_data({}) {}

	constexpr ~BasicString() {
		if (is_on_heap(m_size)) {
			AllocatorType::free_aligned(m_data.heap);
		}
	}

	constexpr BasicString(const T* str) {
		u64 len = toki::strlen(str);
		initialize_based_on_size(len);
		copy_to_buffer(str, len);
	}

	constexpr BasicString(const T* str, u64 size) {
		initialize_based_on_size(size);
		copy_to_buffer(str, size);
	}

	constexpr BasicString(u64 size, T ch = 0) {
		initialize_based_on_size(size);
		auto ptr = get_ptr();
		toki::memset(ptr, size, ch);
		auto a = 0;
	}

	constexpr BasicString(const BasicString& other) {
		initialize_based_on_size(other.size());
		copy_to_buffer(other.get_ptr(), other.m_size);
	}

	constexpr BasicString& operator=(const BasicString& other) {
		if (&other != this) {
			_copy(other);
		}

		return *this;
	}

	constexpr BasicString(BasicString&& other) {
		_swap(*this, other);
	}

	constexpr BasicString& operator=(BasicString&& other) {
		if (&other != this) {
			_copy(other);
		}

		return *this;
	}

	constexpr void resize(u64 new_size) {
		initialize_based_on_size(new_size);
	}

	constexpr u64 size() const {
		return m_size;
	}

	constexpr const T* data() const {
		return get_ptr();
	}

	constexpr T* data() {
		return get_ptr();
	}

	T& operator[](u64 pos) {
		return m_data[pos];
	}

	operator T*() {
		return m_data;
	}

	operator void*() {
		return m_data;
	}

private:
	constexpr void _copy(const BasicString& other) {
		initialize_based_on_size(other.size());
		toki::memcpy(get_ptr(), other.m_data.heap, other.m_size * sizeof(T));
	}

	constexpr void _swap(BasicString&& other) {
		toki::swap(m_data, other.m_data);
	}

private:
	void initialize_based_on_size(u64 len) {
		if (is_on_heap(m_size)) {
			AllocatorType::free_aligned(m_data.heap);
		}

		if (is_on_heap(len)) {
			m_data.heap = static_cast<T*>(AllocatorType::allocate_aligned((len) * sizeof(T), alignof(T)));
		}

		m_size = len;
	}

	b8 is_on_heap(u64 len) const {
		return len > STACK_VS_HEAP_CUTOFF - 1;
	}

	const T* get_ptr() const {
		return is_on_heap(m_size) ? m_data.heap : m_data.stack;
	}

	void copy_to_buffer(const T* str, u64 length) {
		toki::memcpy(get_ptr(), str, (length + 1) * sizeof(T));
		get_ptr()[length] = 0;
	}

	T* get_ptr() {
		return is_on_heap(m_size) ? m_data.heap : m_data.stack;
	}

	b8 is_stack : 1 {};
	u64 m_size : 63 {};
	union {
		T* heap;
		T stack[STACK_VS_HEAP_CUTOFF]{};
	} m_data{};
};

template <CIsAllocator AllocatorType = DefaultAllocator>
using String = BasicString<char, AllocatorType>;
template <CIsAllocator AllocatorType = DefaultAllocator>
using WideString = BasicString<wchar, AllocatorType>;

}  // namespace toki
