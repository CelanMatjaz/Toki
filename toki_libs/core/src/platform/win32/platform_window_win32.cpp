#include <winuser.h>

#include "../platform_window.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <windef.h>
//
#include <Windows.h>

namespace toki {

#define TK_WIN32_WINDOW_CLASS_NAME "TokiEngineWindowClass"

static HINSTANCE win32_instance{};

void window_system_initialize(const WindowSystemInit& init) {
    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = init.window_proc;
    window_class.hInstance = init.instance;
    window_class.lpszClassName = TK_WIN32_WINDOW_CLASS_NAME;

    TK_PLATFORM_ASSERT(RegisterClassA(&window_class) != 0, "Could not register window class", );

    win32_instance = init.instance;
}

void window_system_shutdown() {}

NativeWindowHandle window_create(const char* title, u32 width, u32 height, const WindowInitFlags& flags) {
    HWND handle = CreateWindowA(
        TK_WIN32_WINDOW_CLASS_NAME,
        title,
        WS_OVERLAPPEDWINDOW | (flags.show_on_create ? WS_VISIBLE : 0),
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        0,
        0,
        win32_instance,
        0);

    TK_PLATFORM_ASSERT(handle != 0, "Window was not created");

    return NativeWindowHandle{ .ptr = handle };
}

void window_destroy(NativeWindowHandle handle) {
    DestroyWindow(handle);
}

Vec2<u32> window_get_dimensions(NativeWindowHandle handle) {
    RECT rect{};
    GetWindowRect(handle, &rect);
    return Vec2<u32>(static_cast<u32>(rect.right - rect.left), static_cast<u32>(rect.bottom - rect.top));
}

}  // namespace toki

#endif
