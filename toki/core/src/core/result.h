#pragma once

#include "concepts.h"
#include "types.h"

namespace toki {

template <typename ResultType, typename ErrorType>
	requires(!IsSameValue<ResultType, ErrorType>)
class Result {
	Result() = delete;
	Result(const ResultType&& result): m_value(result) {}
	Result(const ErrorType&& error): m_value(error) {}

	ResultType& result() const {
		return m_value;
	}

	ErrorType& error() const {
		return m_value.error;
	}

	ResultType result_or(ResultType default_value = {}) const {
		return m_isError ? default_value : m_value.result;
	}

	b32 has_error() const {
		return m_isError;
	}

private:
	union {
		ResultType result;
		ErrorType error;
	} m_value;
	b32 m_isError = false;
};

}  // namespace toki
