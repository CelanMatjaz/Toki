#pragma once

#include <toki/core/common/type_traits.h>

#include "toki/core/utils/memory.h"

namespace toki {

class ComptimeString {
public:
	consteval ComptimeString(const char* str): m_data(str), m_size(toki::strlen(str)) {}
	constexpr u32 size() const;
	constexpr const char* data() const;

	constexpr char operator[](u32 index);

private:
	const char* m_data;
	const u32 m_size;
};

constexpr u32 ComptimeString::size() const {
	return m_size;
}

constexpr const char* ComptimeString::data() const {
	return m_data;
}

constexpr char ComptimeString::operator[](u32 index) {
	return m_data[index];
}

}  // namespace toki
