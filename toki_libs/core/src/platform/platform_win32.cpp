#include <winuser.h>

#include "platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "Shlwapi.h"
#include "Windows.h"
#include "core/assert.h"

namespace toki {

namespace platform {

#define MAX_EVENT_PER_LOOP_COUNT 256

void debug_break() {
    DebugBreak();
}

void* memory_allocate(u64 size) {
    void* ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    return ptr;
}

void memory_free(void* ptr) {
    bool _ = HeapFree(GetProcessHeap(), 0, ptr);
}

u64 get_time() {
    FILETIME ft{};
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ull{};
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (ull.QuadPart - 116444736000000000ULL);
}

u64 get_time_microseconds() {
    return get_time() / 10ULL;
}

u64 get_time_milliseconds() {
    return get_time() / 10000ULL;
}

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

    _native_handle.ptr = CreateFileA(path, access_flags, 0, NULL, creation_disposition, attributes, NULL);

    if (flags & STREAM_FLAGS::TRUNC) {
        _offset = SetFilePointerEx(_native_handle.ptr, LARGE_INTEGER{ .QuadPart = 0 }, nullptr, FILE_BEGIN);
    }
}

Stream::~Stream() {
    close();
}

void Stream::seek(STREAM_OFFSET_TYPE offset) {
    static_assert(sizeof(STREAM_OFFSET_TYPE) == 8, "Below function assumes that offset type is int64");
    TK_ASSERT(
        (_offset = SetFilePointerEx(_native_handle.ptr, LARGE_INTEGER{ .QuadPart = offset }, nullptr, FILE_BEGIN)) != 0,
        "Could not set file pointer");
}

Stream::STREAM_OFFSET_TYPE Stream::tell() const {
    return _offset;
}

void Stream::close() {
    if (_native_handle.ptr != nullptr) {
        CloseHandle(_native_handle.ptr);
        _native_handle.ptr = nullptr;
    }
}

extern HINSTANCE win32_instance;

void init_win32(HINSTANCE instance, LRESULT (*window_proc)(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param)) {
    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = TK_WIN32_WINDOW_CLASS_NAME;

    TK_PLATFORM_ASSERT(RegisterClassA(&window_class) != 0, "Could not register window class", );

    win32_instance = instance;
}

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height) {
    HWND handle = CreateWindowA(
        TK_WIN32_WINDOW_CLASS_NAME,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        0,
        0,
        win32_instance,
        0);

    TK_PLATFORM_ASSERT(handle != 0, "Window was not created");

    return NATIVE_HANDLE_TYPE{ .ptr = handle };
}

void window_destroy(NATIVE_HANDLE_TYPE handle) {
    DestroyWindow(handle);
}

}  // namespace platform

}  // namespace toki

#endif
