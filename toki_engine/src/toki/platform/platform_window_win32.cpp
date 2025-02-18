#include "platform_window.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "windows.h"

namespace toki {

namespace platform {

LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_CHILDACTIVATE: {
        }
        case WM_CLOSE: {
        }

        default:
            return DefWindowProcA(handle, msg, w_param, l_param);
            break;
    }
}

HINSTANCE global_instance{};
const char* global_window_class = "TokiWindowClass";

Window create_window(const char* title) {
    Window window{};

    window.handle.ptr = CreateWindowExA(
        0,
        global_window_class,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        global_instance,
        0);

    return window;
}

void poll_events() {
    MSG msg;

    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

}  // namespace platform

}  // namespace toki

#endif
