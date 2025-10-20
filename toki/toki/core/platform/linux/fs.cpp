#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "toki/core/common/assert.h"
#include "toki/core/platform/syscalls.h"

namespace toki {

toki::Expected<NativeHandle, TokiError> open(const char* filename, FileMode mode, u32 flags) {
	int linux_flags = 0;
	if (flags & FileFlags::FILE_FLAG_CREATE) {
		linux_flags |= O_CREAT;
	}

	if (flags & FileFlags::FILE_FLAG_APPEND) {
		linux_flags |= O_APPEND;
	}

	if (flags & FileFlags::FILE_FLAG_TRUNCATE) {
		linux_flags |= O_TRUNC;
	}

	switch (mode) {
		case FileMode::READ:
			linux_flags |= O_RDONLY;
			break;
		case FileMode::WRITE:
			linux_flags |= O_WRONLY;
			break;
		case FileMode::RDWR:
			linux_flags |= O_RDWR;
			break;
	}

	i64 result = ::open(filename, linux_flags, 0777);  // All permissions for now
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return toki::Unexpected(TokiError::FileOpen);
	}

	return static_cast<NativeHandle>(result);
}

toki::Optional<TokiError> close(NativeHandle handle) {
	i64 result = ::close(handle.handle);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return toki::Optional{ TokiError::Unknown };
	}

	return toki::NullOpt{};
}

toki::Expected<i64, TokiError> set_file_pointer(NativeHandle handle, i64 position, FileCursorStart start_from) {
	int whence = 0;
	switch (start_from) {
		case FileCursorStart::BEGIN:
			whence = SEEK_SET;
			break;
		case FileCursorStart::CURRENT:
			whence = SEEK_CUR;
			break;
		case FileCursorStart::END:
			whence = SEEK_END;
			break;
	}

	i64 result = ::lseek(handle.handle, position, whence);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return TokiError::Unknown;
	}

	return static_cast<u64>(result);
}

toki::Expected<u64, TokiError> get_file_pointer(NativeHandle handle) {
	i64 result = ::lseek(handle.handle, 0, SEEK_CUR);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return TokiError::Unknown;
	}

	return static_cast<u64>(result);
}

}  // namespace toki
