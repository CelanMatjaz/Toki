#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/common.h>
#include <toki/core/common/macros.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/types.h>

namespace toki {

template <typename T>
struct Unexpected {
public:
	Unexpected() = delete;
	Unexpected(T&& value): m_value(value) {}
	~Unexpected() = default;

	T m_value;
};

template <typename ExpectedType, typename UnexpectedType>
	requires CIsDifferent<ExpectedType, UnexpectedType> && (!CIsReference<ExpectedType>) &&
			 (!CIsReference<UnexpectedType>)
class Expected {
public:
	Expected(ExpectedType&& expected): m_isError(false) {
		construct_at(&m_values.expected, forward<ExpectedType>(expected));
	}
	Expected(UnexpectedType&& unexpected): m_isError(true) {
		construct_at(&m_values.unexpected, forward<UnexpectedType>(unexpected));
	}
	Expected(Unexpected<UnexpectedType>&& unexpected): m_isError(true) {
		construct_at(&m_values.unexpected, forward<UnexpectedType>(unexpected.m_value));
	}

	~Expected() {
		if (m_isError) {
			toki::destroy_at(&m_values.unexpected);
		} else {
			toki::destroy_at(&m_values.expected);
		}
	}

	ExpectedType& value() const {
		if (m_isError) {
			TK_UNREACHABLE();
		}
		return m_values.expected;
	}

	ExpectedType& value() {
		if (m_isError) {
			TK_UNREACHABLE();
		}
		return m_values.expected;
	}

	ExpectedType& value_or(const ExpectedType& value = {}) {
		if (m_isError) {
			return value;
		}
		return m_values.expected;
	}

	ExpectedType& value_or(ExpectedType&& value = {}) {
		if (m_isError) {
			return toki::remove_r_value_ref(const_cast<ExpectedType&&>(value));
		}
		return m_values.expected;
	}

	UnexpectedType& error() {
		if (!m_isError) {
			TK_UNREACHABLE();
		}
		return m_values.unexpected;
	}

	toki::b8 is_error() const {
		return m_isError;
	}

	operator b8() {
		return !m_isError;
	}

	operator ExpectedType&() {
		if (m_isError) {
			TK_UNREACHABLE();
		}
		return m_values.expected;
	}

	operator UnexpectedType() {
		if (!m_isError) {
			TK_UNREACHABLE();
		}
		return m_values.unexpected;
	}

private:
	union Values {
		ExpectedType expected;
		UnexpectedType unexpected;

		constexpr Values() {}
		constexpr ~Values() {}
	} m_values;
	b8 m_isError;
};

}  // namespace toki
