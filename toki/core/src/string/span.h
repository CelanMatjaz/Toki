#pragma once

#include "../core/types.h"

namespace toki {

template <typename T>
class Span {
public:
	Span(const T* data, u64 length): m_data(const_cast<T*>(data)), m_length(length) {}
	Span(T* data, u64 length): m_data(data), m_length(length) {}

	inline T* data() const {
		return m_data;
	}

	inline u64 size() const {
		return m_length;
	}

	inline operator T*() const {
		return m_data;
	}

	inline operator b8() const {
		return m_data != nullptr && m_length != 0;
	}

private:
	T* m_data;
	const u64 m_length;
};

}  // namespace toki
