#include "toki/core/platform/syscalls.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <toki/core/common/assert.h>
#include <toki/core/platform/linux/defines.h>
#include <toki/core/platform/linux/linux.h>
#include <unistd.h>

namespace toki {

toki::Expected<void*, TokiError> allocate(u64 size) {
	i64 result = ::syscall(
		Syscalls::MMAP, 0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (result < 0) {
		switch (-result) {
			default:
				return toki::Unexpected(TokiError{ Error::MEMORY_ALLOCATE, -result });
		}

		TK_UNREACHABLE();
	}

	u64* ptr = reinterpret_cast<u64*>(result);
	*ptr	 = size;

	return reinterpret_cast<void*>(*ptr);
}

TokiError free(void* ptr) {
	TK_ASSERT(ptr != nullptr);

	u64 size = *(reinterpret_cast<u64*>(ptr) - 1);

	i64 result = ::syscall(Syscalls::MUNMAP, ptr, size);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::MEMORY_FREE, -result };
		}

		TK_UNREACHABLE();
	}

	return TokiError{};
}

toki::Expected<u64, TokiError> write(NativeHandle handle, const void* data, u64 size) {
	i64 result = ::syscall(Syscalls::WRITE, static_cast<i32>(handle), data, size);
	if (result < 0) {
		switch (-result) {
			default:
				return toki::Unexpected(TokiError{ Error::FILE_WRITE, -result });
		}

		TK_UNREACHABLE();
	}

	return static_cast<u64>(result);
}

toki::Expected<u64, TokiError> read(NativeHandle handle, void* data, u64 size) {
	i64 result = ::syscall(Syscalls::READ, static_cast<i32>(handle), data, size);
	if (result < 0) {
		switch (-result) {
			default:
				return toki::Unexpected(TokiError{ Error::FILE_READ, -result });
		}

		TK_UNREACHABLE();
	}

	return static_cast<u64>(result);
}

toki::Expected<NativeHandle, TokiError> open(const char* filename, FileMode mode, u32 flags /* FileFlags */) {
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

	i64 result = ::syscall(Syscalls::OPEN, filename, linux_flags, 0777);  // All permissions for now
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::FILE_OPEN, -result };
		}

		TK_UNREACHABLE();
	}

	return static_cast<NativeHandle>(result);
}

TokiError close(NativeHandle handle) {
	i64 result = ::syscall(Syscalls::CLOSE, handle);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::FILE_CLOSE, -result };
		}

		TK_UNREACHABLE();
	}

	return TokiError{};
}

toki::Expected<i64, TokiError> seek(NativeHandle handle, i64 position, FileCursorStart start_from) {
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

	i64 result = ::syscall(Syscalls::LSEEK, handle, position, whence);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::FILE_SEEK, -result };
		}

		TK_UNREACHABLE();
	}

	return result;
}

toki::Expected<u64, TokiError> tell(NativeHandle handle) {
	i64 result = ::syscall(Syscalls::LSEEK, handle, 0, SEEK_CUR);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::FILE_TELL, -result };
		}

		TK_UNREACHABLE();
	}

	return static_cast<u64>(result);
}

toki::Expected<u64, TokiError> get_current_time() {
	struct timespec ts{};
	i64 result = ::syscall(Syscalls::CLOCK_GETTIME, CLOCK_REALTIME, &ts);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::TIME_GET, -result };
		}

		TK_UNREACHABLE();
	}

	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

TokiError sleep(u32 millis) {
	struct timespec ts{};
	ts.tv_sec  = millis / 1000;
	ts.tv_nsec = (millis % 1000) * 1000000;

	i64 result = ::syscall(Syscalls::NANOSLEEP, &ts, nullptr);
	if (result < 0) {
		switch (-result) {
			default:
				return TokiError{ Error::THREAD_SLEEP, -result };
		}

		TK_UNREACHABLE();
	}

	return TokiError{};
}

}  // namespace toki
