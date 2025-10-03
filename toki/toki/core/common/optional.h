#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/common.h>
#include <toki/core/types.h>

namespace toki {

struct NullOpt {};

template <typename OptionalType>
class Optional {
public:
	Optional(): m_hasValue(false) {}
	Optional(NullOpt): m_hasValue(false) {}
	Optional(OptionalType&& expected): m_hasValue(true) {
		construct_at(&m_value, forward<OptionalType>(expected));
	}

	~Optional() {
		if (m_hasValue) {
			toki::destroy_at(&m_value);
		}
	}

	OptionalType& value() {
		if (!m_hasValue) {
			TK_UNREACHABLE();
		}
		return m_value;
	}

	const OptionalType& value() const {
		if (!m_hasValue) {
			TK_UNREACHABLE();
		}
		return m_value;
	}

	OptionalType& value_or(const OptionalType& value = {}) {
		if (m_hasValue) {
			return value;
		}
		return m_value;
	}

	OptionalType& value_or(OptionalType&& value = {}) {
		if (m_hasValue) {
			return toki::remove_r_value_ref(const_cast<OptionalType&&>(value));
		}
		return m_value;
	}

	toki::b8 has_value() const {
		return m_hasValue;
	}

	operator b8() const {
		return m_hasValue;
	}

	operator OptionalType&() {
		if (!m_hasValue) {
			TK_UNREACHABLE();
		}
		return m_value;
	}

private:
	OptionalType m_value;
	b8 m_hasValue;
};

}  // namespace toki
