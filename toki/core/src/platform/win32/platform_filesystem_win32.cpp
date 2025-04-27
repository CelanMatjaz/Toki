#if defined(TK_PLATFORM_WINDOWS)

#include <Shlwapi.h>
#include <Windows.h>

namespace toki {

void file_delete(PATH_TYPE path) {
    TK_ASSERT(DeleteFileA(path) != 0, "Could not delete file, platform specific error code: %lu", GetLastError());
}

b8 file_exists(PATH_TYPE path) {
    return PathFileExistsA(path);
}

b8 directory_exists(PATH_TYPE path) {
    return file_exists(path);
}

Stream::Stream(const char* path, u32 flags) {
    DWORD creation_disposition = 0;
    if (flags & STREAM_FLAGS::TRUNC) {
        creation_disposition = CREATE_ALWAYS;
    }

    DWORD access_flags = 0;
    if (flags & STREAM_FLAGS::INPUT) {
        access_flags |= GENERIC_READ;
        // Expect the file to already exist, since we want to read from it
        creation_disposition = OPEN_EXISTING;
    }
    if (flags & STREAM_FLAGS::OUTPUT) {
        access_flags |= GENERIC_WRITE;
    }

    DWORD attributes = FILE_ATTRIBUTE_NORMAL;

    m_NativeHandle.ptr = CreateFileA(path, access_flags, 0, NULL, creation_disposition, attributes, NULL);

    if (flags & STREAM_FLAGS::TRUNC) {
        m_Offset = SetFilePointerEx(m_NativeHandle.ptr, LARGE_INTEGER{ .QuadPart = 0 }, nullptr, FILE_BEGIN);
    }
}

Stream::~Stream() {
    close();
}

void Stream::seek(STREAM_OFFSET_TYPE offset) {
    static_assert(sizeof(STREAM_OFFSET_TYPE) == 8, "Below function assumes that offset type is int64");
    TK_ASSERT(
        (m_Offset = SetFilePointerEx(m_NativeHandle.ptr, LARGE_INTEGER{ .QuadPart = offset }, nullptr, FILE_BEGIN)) !=
            0,
        "Could not set file pointer");
}

Stream::STREAM_OFFSET_TYPE Stream::tell() {
    return m_Offset;
}

void Stream::close() {
    if (m_NativeHandle.ptr != nullptr) {
        CloseHandle(m_NativeHandle.ptr);
        m_NativeHandle.ptr = nullptr;
    }
}

}  // namespace toki

#endif
