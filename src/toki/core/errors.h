#pragma once

#include <toki/core/types.h>

namespace toki {

enum struct Error {
	NO_ERROR,
	UNKNOWN,

	MEMORY_ALLOCATE,
	MEMORY_FREE,

	FILE_OPEN,
	FILE_CLOSE,
	FILE_SEEK,
	FILE_TELL,
	FILE_WRITE,
	FILE_READ,

	TIME_GET,

	THREAD_SLEEP,

	MEMORY_ALLOCATION_FAILED,
};

class TokiError {
public:
	TokiError(Error error = Error::NO_ERROR, i64 os_error = 0): m_error(error), m_osError(static_cast<u32>(os_error)) {}

	b8 ok() {
		return m_error == Error::NO_ERROR && m_osError == 0;
	}

	Error error() const {
		return m_error;
	}

	u32 os_error() const {
		return m_osError;
	}

private:
	Error m_error = Error::NO_ERROR;
	u32 m_osError = 0;
};

}  // namespace toki
