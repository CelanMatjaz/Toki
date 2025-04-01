#include "../platform_filesystem.h"

#if defined(TK_PLATFORM_LINUX)

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace toki {

void file_delete(PATH_TYPE path) {
    syscall(SYS_unlink, path);
}

b8 file_exists(PATH_TYPE path) {
    i64 result = syscall(SYS_access, path, F_OK);
    return result == 0;
}

b8 directory_exists(PATH_TYPE path) {
    struct stat st{};
    u64 result = syscall(SYS_stat, path, &st);

    if (result == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }

    return false;
}

Stream::Stream(PATH_TYPE path, u32 flags) {
    i32 internal_flags = 0;

    if (flags & STREAM_FLAGS::TRUNC) {
        internal_flags |= O_TRUNC;
    }

    if (flags & STREAM_FLAGS::ATE) {
        internal_flags |= O_APPEND;
    }

    if (flags & STREAM_FLAGS::OUTPUT && flags & STREAM_FLAGS::INPUT) {
        internal_flags |= O_RDWR;
    } else if (flags & STREAM_FLAGS::INPUT) {
        internal_flags |= O_RDONLY;
    } else if (flags & STREAM_FLAGS::OUTPUT) {
        internal_flags |= O_WRONLY;
    }

    i64 fd = syscall(SYS_openat, AT_FDCWD, path, internal_flags);
    if (fd < 0) {
        TK_ASSERT(false, "File %lli not opened", fd);
    }

    m_NativeHandle.i64 = fd;
}

Stream::~Stream() {
    close();
}

void Stream::seek(STREAM_OFFSET_TYPE offset) {
    static_assert(sizeof(STREAM_OFFSET_TYPE) == 8, "Below function assumes that offset type is int64");

    m_Offset = syscall(SYS_lseek, m_NativeHandle.i64, offset, SEEK_SET);
    TK_ASSERT(m_Offset != -1, "Could not set file cursor");
}

Stream::STREAM_OFFSET_TYPE Stream::tell() {
    m_Offset = syscall(SYS_lseek, m_NativeHandle.i64, 0, SEEK_CUR);
    TK_ASSERT(m_Offset != -1, "Could not get file cursor");
    return m_Offset;
}

void Stream::close() {
    if (m_NativeHandle.i64 != 0) {
        syscall(SYS_close, m_NativeHandle.i64);
    }
}

}  // namespace toki

#endif
