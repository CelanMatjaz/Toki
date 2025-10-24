#pragma once

#include <toki/core/types.h>

namespace toki {

struct Handle {
	Handle(): m_value(0) {}
	Handle(u64 value): m_value(value) {}

	u64 m_value{};

	operator u64() const {
		return m_value;
	}

	operator b8() const {
		return m_value != 0;
	}
};

}  // namespace toki
