#pragma once

#include "../containers/dynamic_array.h"
#include "../core/core.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Windows.h>
#define TK_WIN32_WINDOW_CLASS_NAME "TokiEngineWindowClass"

#elif defined(TK_PLATFORM_LINUX)

#include <wayland-client.h>

#endif

#include "platform_.h"

namespace toki {

namespace platform {

using PATH_TYPE = const char*;

union NATIVE_HANDLE_TYPE {
    void* ptr;
    i64 i64;

#if defined(TK_PLATFORM_WINDOWS)
    operator HWND() {
        return reinterpret_cast<HWND>(ptr);
    }
#endif
};

#if defined(TK_PLATFORM_WINDOWS)
void init_win32(HINSTANCE instance, LRESULT (*window_proc)(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param));
inline HINSTANCE win32_instance;
#elif defined(TK_PLATFORM_LINUX)
void initialize_wayland();
void shutdown_wayland();
inline wl_display* wayland_display{};
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
    Stream(PATH_TYPE path, u32 flags);
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
    STREAM_OFFSET_TYPE tell();
    void close();

    void swap(Stream&& other) {
        m_Offset = other.m_Offset;
        m_NativeHandle = other.m_NativeHandle;
        other.m_NativeHandle.i64 = 0;
    }

private:
    STREAM_OFFSET_TYPE m_Offset{ 0 };
    NATIVE_HANDLE_TYPE m_NativeHandle{};
};

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height);

void window_destroy(NATIVE_HANDLE_TYPE handle);

}  // namespace platform

}  // namespace toki
