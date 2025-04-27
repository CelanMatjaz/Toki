#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "../../core/assert.h"
#include "../file.h"
#include "../platform.h"

namespace toki {

void File::open(const char* path, u32 flags) {
    TK_ASSERT(!_is_open, "Trying to open an file that already has a file handle open");

    i32 open_flags = 0;
    if (flags & FileOpenFlags::FILE_OPEN_RDWR) {
        open_flags = O_RDWR;
    } else if (flags & FileOpenFlags::FILE_OPEN_WRITE) {
        open_flags = O_WRONLY;
    } else {
        open_flags = O_RDONLY;
    }

    if (flags & FileOpenFlags::FILE_OPEN_APPEND) {
        open_flags |= O_APPEND;
    }

    if (flags & FileOpenFlags::FILE_OPEN_TRUNC) {
        open_flags |= O_TRUNC;
    }

    open_flags |= O_CREAT;

    _handle = syscall(SYS_open, path, open_flags);
    TK_ASSERT_PLATFORM_ERROR(_handle, "Error opening fd");
    _is_open = true;
}

void File::close() {
    if (_is_open) {
        TK_ASSERT_PLATFORM_ERROR(syscall(SYS_close, _handle), "Error closing fd");
        _is_open = false;
    }
}

void File::write(u32 n, const void* data) {
    i32 written_byte_count = syscall(SYS_write, _handle, data, n);
    TK_ASSERT_PLATFORM_ERROR(written_byte_count, "Error writing to fd");
}

void File::read(u32 n, void* data) {
    i32 read_byte_count = syscall(SYS_read, _handle, data, n);
    TK_ASSERT_PLATFORM_ERROR(read_byte_count, "Error reading from fd");
}

}  // namespace toki
