#include "platform_window.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "Windows.h"

namespace toki {

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

}  // namespace toki

#endif
