#pragma once

#include "../containers/dynamic_array.h"
#include "../core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"

#define TK_WIN32_WINDOW_CLASS_NAME "TokiEngineWindowClass"

#endif

#include "platform_.h"

namespace toki {

namespace platform {

using PATH_TYPE = const char*;

union NATIVE_HANDLE_TYPE {
    void* ptr;
    u64 u64;

#if defined(TK_PLATFORM_WINDOWS)
    operator HWND() {
        return reinterpret_cast<HWND>(ptr);
    }
#endif
};

#if defined(TK_PLATFORM_WINDOWS)
void init_win32(HINSTANCE instance, LRESULT (*window_proc)(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param));
inline HINSTANCE win32_instance;
#endif

u64 get_time_microseconds();

u64 get_time_milliseconds();

void file_delete(PATH_TYPE path);

b8 file_exists(PATH_TYPE path);

b8 directory_exists(PATH_TYPE path);

class Stream {
public:
    using STREAM_OFFSET_TYPE = i64;

    enum STREAM_FLAGS : u32 {
        INPUT = BIT(0),
        OUTPUT = BIT(1),
        ATE = BIT(2),
        TRUNC = BIT(3),
    };

    Stream() = delete;
    Stream(const char* path, u32 flags);
    ~Stream();

    DELETE_COPY(Stream);

    Stream(Stream&& other) {
        swap(MOVE(other));
    }

    Stream& operator=(Stream&& other) {
        if (this != &other) {
            swap(MOVE(other));
        }
        return *this;
    }

    void seek(STREAM_OFFSET_TYPE offset);
    STREAM_OFFSET_TYPE tell() const;
    void close();

    void swap(Stream&& other) {
        _offset = other._offset;
        _native_handle = other._native_handle;
        other._native_handle.u64 = 0;
    }

private:
    STREAM_OFFSET_TYPE _offset{0};
    NATIVE_HANDLE_TYPE _native_handle{};
};

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height);

void window_destroy(NATIVE_HANDLE_TYPE handle);

}  // namespace platform

}  // namespace toki
