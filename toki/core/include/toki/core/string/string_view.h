#pragma once

#include <toki/core/string/basic_string.h>

#include "toki/core/utils/memory.h"

namespace toki {

template <typename T>
class BasicStringView {
public:
	BasicStringView() = delete;

	constexpr BasicStringView(T* str): m_size(toki::strlen(str)), m_ptr(str) {}

	constexpr BasicStringView(const T* str): m_size(toki::strlen(str)), m_ptr(str) {}

	constexpr BasicStringView(const BasicString<T> str): m_size(str.size()), m_ptr(str.data()) {}

	constexpr BasicStringView(const BasicStringView& other): m_size(other.m_size), m_ptr(other.m_size) {}

	constexpr BasicStringView& operator=(const BasicStringView& other) {
		if (other == this) {
			return this;
		}

		m_size = other.m_size;
		m_ptr = other.m_ptr;

		return this;
	}

	constexpr BasicStringView(BasicStringView&& other): m_size(other.size), m_ptr(other.m_ptr) {}

	constexpr BasicStringView& operator=(BasicStringView&& other) {
		if (other == this) {
			return this;
		}

		m_size = other.m_size;
		m_ptr = other.m_ptr;

		return this;
	}

	constexpr u32 size() const {
		return m_size;
	}

	constexpr const T* data() const {
		return m_ptr;
	}

private:
	u32 m_size = 0;
	const T* m_ptr = nullptr;
};

using StringView = BasicStringView<i8>;
using WideStringView = BasicStringView<i32>;

}  // namespace toki
