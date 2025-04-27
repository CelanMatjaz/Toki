#pragma once

#include "string.h"

namespace toki {

class StringView {
public:
	StringView(const char* str): m_data(str), m_length(toki::strlen(str)) {}
	StringView(const char* str, u32 length): m_data(str), m_length(length) {}

	inline const char* data() const {
		return m_data;
	}

	inline u64 length() const {
		return m_length;
	}

	inline operator const char*() const {
		return m_data;
	}

private:
	const char* m_data;
	const u64 m_length;
};

}  // namespace toki
