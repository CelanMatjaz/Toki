#include "platform/platform_io.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Shlobj.h>
#include <pathcch.h>
#include <windows.h>

#include "core/assert.h"
#include "core/common.h"

namespace toki {

namespace platform {

File file_open(PATH path, u32 open_flags, u32 create_attribute_flags) {
    HANDLE file_handle = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    TK_ASSERT(file_handle != INVALID_HANDLE_VALUE, "File not opened");

    DWORD access_flags = 0;
    if (open_flags & FileOpen_Read) {
        access_flags |= GENERIC_READ;
    }
    if (open_flags & FileOpen_Write) {
        access_flags |= GENERIC_WRITE;
    }

    DWORD creation_disposition = CREATE_ALWAYS;
    if (open_flags & FileOpen_OpenExisting) {
        creation_disposition = OPEN_EXISTING;
    }
    if (open_flags & FileOpen_CreateNew) {
        creation_disposition = CREATE_ALWAYS;
    }

    DWORD attribute_flags = FILE_ATTRIBUTE_NORMAL;
    if (create_attribute_flags & FileCreateAttribute_Hidden) {
        attribute_flags |= FILE_ATTRIBUTE_HIDDEN;
    }
    if (create_attribute_flags & FileCreateAttribute_ReadOnly) {
        attribute_flags |= FILE_ATTRIBUTE_READONLY;
    }

    File file{};
    file.native_handle = CreateFileW(path, access_flags, 0, NULL, creation_disposition, attribute_flags, NULL);

    return file;
}

void file_close(File* file) {
    TK_ASSERT(file->native_handle != INVALID_HANDLE_VALUE, "File with invalid handle provided");
}

void file_delete(PATH path) {
    DeleteFileW(path);
}

bool file_exists(PATH path) {
    WIN32_FIND_DATAW data;
    return FindFirstFileW(path, &data) != INVALID_HANDLE_VALUE;
}

void directory_create(PATH path) {
    wchar_t buffer[MAX_PATH]{};
    u32 buffer_offset = GetCurrentDirectoryW(sizeof(buffer), buffer);

    wchar_t normalized_path[MAX_PATH]{};
    normalize_path(path, normalized_path, sizeof(normalized_path));

    buffer[buffer_offset++] = '\\';
    TK_ASSERT(strlen(path) + buffer_offset <= sizeof(buffer), "Provided path is too long");
    for (u32 path_offset = 0; (buffer[buffer_offset++] = path[path_offset++]);) {}

    SHCreateDirectoryExW(nullptr, buffer, nullptr);
}

void normalize_path(const wchar_t* path, wchar_t* path_out, u32 path_out_size) {
    PathCchCanonicalize(path_out, path_out_size, path);
}

}  // namespace platform

}  // namespace toki

#endif
