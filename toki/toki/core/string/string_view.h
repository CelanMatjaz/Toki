#pragma once

#include <toki/core/string/basic_string.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T>
class BasicStringView {
public:
	BasicStringView() = default;

	constexpr BasicStringView(const T* str): m_ptr(str), m_size(toki::strlen(str)) {}

	constexpr BasicStringView(const T* str, u64 length): m_ptr(str), m_size(length) {}

	constexpr BasicStringView(const BasicString<T> str): m_ptr(str.data()), m_size(str.size()) {}

	constexpr BasicStringView(const BasicStringView& other): m_ptr(other.m_ptr), m_size(other.m_size) {}

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
		if (&other == this) {
			return *this;
		}

		m_size = other.m_size;
		m_ptr = other.m_ptr;

		return *this;
	}

	constexpr u64 size() const {
		return m_size;
	}

	constexpr const T* data() const {
		return m_ptr;
	}

	constexpr toki::String to_string() const {
		return toki::String{ m_ptr, m_size };
	}

private:
	const T* m_ptr{};
	u64 m_size{};
};

using StringView = BasicStringView<char>;
using WideStringView = BasicStringView<wchar>;

}  // namespace toki
