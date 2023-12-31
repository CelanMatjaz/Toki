#include "windows_window.h"

#ifdef WINDOW_SYSTEM_WINDOWS

#include "core/application.h"
#include "core/assert.h"
#include "events/events.h"
#include "windows.h"
#include "winuser.h"

namespace Toki {

WindowsWindow::WindowsWindow(const WindowConfig& config) {
    HINSTANCE hInstance = GetModuleHandle(0);

    if (!isWindowClassRegistered) {
        WNDCLASS wc{};
        wc.lpfnWndProc = windowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = windowClassName;

        RegisterClass(&wc);

        isWindowClassRegistered = true;
    }

    RECT lpRect = { 0, 0, config.width, config.height };
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    BOOL menu = false;
    AdjustWindowRect(&lpRect, windowStyle, menu);

    handle = CreateWindowEx(
        0,
        windowClassName,
        config.title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        lpRect.right - lpRect.left,
        lpRect.bottom - lpRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    TK_ASSERT(handle != nullptr, "Window handle is nullptr");

    ShowWindow(handle, true);
}

WindowsWindow::~WindowsWindow() {}

bool WindowsWindow::shouldClose() const {
    return shouldWindowClose;
}

void WindowsWindow::pollEvents() {
    MSG msg{};
    while (!shouldWindowClose && GetMessage(&msg, handle, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WindowsWindow::close() {
    shouldWindowClose = true;
    DestroyWindow(handle);
}

Window::WindowDimensions WindowsWindow::getDimensions() const {
    return { width, height };
}

void* WindowsWindow::getHandle() const {
    return handle;
}

LRESULT CALLBACK WindowsWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_ACTIVATEAPP: {
            if (wParam)
                Application::get().onEvent(Event(EventType::WindowFocus));
            else
                Application::get().onEvent(Event(EventType::WindowBlur));
            break;
        }
        case WM_MOVE: {
            Application::get().onEvent(Event(EventType::WindowMove, LOWORD(lParam) + (HIWORD(lParam) << 16)));
            break;
        }
        case WM_SIZE: {
            switch (wParam) {
                case SIZE_RESTORED: {
                    Application::get().onEvent(Event(EventType::WindowResize, LOWORD(lParam) + (HIWORD(lParam) << 16)));
                    break;
                }
                case SIZE_MAXIMIZED: {
                    Application::get().onEvent(Event(EventType::WindowMaximize));
                    Application::get().onEvent(Event(EventType::WindowResize, LOWORD(lParam) + (HIWORD(lParam) << 16)));
                    break;
                }
                case SIZE_MINIMIZED: {
                    Application::get().onEvent(Event(EventType::WindowMinimize));
                    Application::get().onEvent(Event(EventType::WindowResize, LOWORD(lParam) + (HIWORD(lParam) << 16)));
                    break;
                }
            }

            break;
        }
        //case WM_QUIT:                         
        case WM_CLOSE: {
            Application::get().onEvent(Event(EventType::WindowClose));
            break;
        }
        default: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return TRUE;
}

}  // namespace Toki

#endif
