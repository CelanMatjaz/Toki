#include "window.h"

namespace toki {

void Window::create(const Config& config) {
    mNativeHandle = window_create(config.title, config.width, config.height);
}

void Window::destroy() {
    window_destroy(mNativeHandle);
    *this = {};
}

#if defined(TK_PLATFORM_WINDOWS)

#include <winuser.h>

void Window::handle_events(EventHandler& handler) {
    Event e{};
    handler.handle(e);
    MSG msg{};

    while (PeekMessageA(&msg, m_native_handle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

LRESULT window_proc(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
        } break;
        case WM_MOUSEMOVE: {
            printf("WM_MOUSEMOVE\n");
        } break;
        case WM_MOUSEHOVER: {
            printf("WM_MOUSEHOVER\n");
        } break;
        case WM_MOUSELEAVE: {
            printf("WM_MOUSELEAVE\n");
        } break;
        case WM_LBUTTONDOWN: {
            SetCapture(handle);
        } break;

        case WM_MOVE: {
            i32 x = LOWORD(l_param);
            i32 y = HIWORD(l_param);
            Event _ = Event::create(EventType::WindowMove, EventData{ .window_move_event_data{ .x = x, .y = y } });

            printf("WM_MOVE %i %i\n", x, y);
        } break;

        case WM_LBUTTONUP: {
            ReleaseCapture();
        } break;
    }

    return DefWindowProc(handle, msg, w_param, l_param);
}
#endif

}  // namespace toki
