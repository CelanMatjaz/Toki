#include "tkpch.h"
#include "windows_window.h"
#include "core/assert.h"
#include "core/engine.h"
#include "events/events.h"
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Toki {
    TokiWindowsWindow::TokiWindowsWindow(const WindowConfig& windowConfig, void* engine) : Window(windowConfig, engine) {
        TokiWindowsWindow::engine = engine;

        HMODULE hInstance = GetModuleHandle(nullptr);
        auto console = GetConsoleWindow();

        if (!isClassRegistered) {
            WNDCLASS windowClass = { };
            windowClass.lpfnWndProc = WindowProc;
            windowClass.hInstance = hInstance;
            windowClass.lpszClassName = WINDOW_CLASS_NAME;
            windowClass.style = CS_DBLCLKS;

            RegisterClass(&windowClass);

            isClassRegistered = true;
        }

        HMENU menu;
        DWORD windowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        if (windowConfig.resizable) windowStyle |= WS_THICKFRAME;

        handle = CreateWindowEx(
            0,
            WINDOW_CLASS_NAME,
            windowConfig.title.c_str(),
            windowStyle, // TODO: add support for customization
            CW_USEDEFAULT, CW_USEDEFAULT, windowConfig.width, windowConfig.height,

            console, // TODO: add condition for console app and window app
            nullptr, // TODO: add support for custom menu
            hInstance,
            nullptr
        );

        TK_ASSERT(handle != nullptr, "Handle not created");
    }

    TokiWindowsWindow::~TokiWindowsWindow() {

    }

    void TokiWindowsWindow::showWindow() {
        ShowWindow(handle, SW_SHOW);
    }

    void TokiWindowsWindow::pollEvents() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                shouldWindowClose = true;
                return;
            }
        }
    }

    uint8_t getKeyMods() {
        uint8_t mods = 0;
        mods |= GetAsyncKeyState(VK_LSHIFT) << LEFT_SHIFT;
        mods |= GetAsyncKeyState(VK_LCONTROL) << LEFT_CTRL;
        mods |= GetAsyncKeyState(VK_LMENU) << LEFT_MENU;
        mods |= GetAsyncKeyState(VK_RSHIFT) << RIGHT_SHIFT;
        mods |= GetAsyncKeyState(VK_RCONTROL) << RIGHT_CTRL;
        mods |= GetAsyncKeyState(VK_RMENU) << RIGHT_MENU;
        return mods;
    }

    MouseButton mapMsgToButton(uint32_t msg) {
        switch (msg) {
            case WM_LBUTTONDOWN: return MouseButton::Left;
            case WM_RBUTTONDOWN: return MouseButton::Right;
            case WM_MBUTTONDOWN: return MouseButton::Middle;
            case WM_LBUTTONUP: return MouseButton::Left;
            case WM_RBUTTONUP: return MouseButton::Right;
            case WM_MBUTTONUP: return MouseButton::Middle;
            case WM_LBUTTONDBLCLK: return MouseButton::Left;
            case WM_RBUTTONDBLCLK: return MouseButton::Right;
            case WM_MBUTTONDBLCLK: return MouseButton::Middle;
        }
    }

    LRESULT CALLBACK TokiWindowsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
            return true;


        switch (uMsg) {
            case WM_SIZE: {
                RECT dimensions{};
                GetWindowRect(hwnd, &dimensions);
                ((Engine*) TokiWindowsWindow::engine)->getWindow()->setDimensions(dimensions.right - dimensions.left, dimensions.bottom - dimensions.top);

                switch (wParam) {
                    case SIZE_RESTORED: {
                        WindowResizeEvent e((int16_t) (lParam >> 16), (int16_t) lParam);
                        ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                        break;
                    }
                    case SIZE_MAXIMIZED: {
                        WindowMinimizeEvent e;
                        ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                        break;
                    }
                    case SIZE_MINIMIZED: {
                        WindowMaximizeEvent e;
                        ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                        break;
                    }
                }
                break;
            }
            case WM_MOUSEACTIVATE:
            case WM_SETFOCUS: {
                WindowFocusEvent e;
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_KEYDOWN: {
                uint16_t repeatCount = lParam;
                if (repeatCount) {
                    KeyRepeatEvent e(wParam, (lParam >> 16) & 0b1111111, getKeyMods(), repeatCount);
                    ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                    break;
                }
                KeyPressEvent e(wParam, (lParam >> 16) & 0b1111111, getKeyMods());
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_KEYUP: {
                KeyReleaseEvent e(wParam, (lParam >> 16) & 0b1111111, getKeyMods());
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_LBUTTONDBLCLK: // TODO: find out why double clicking is not working, check style flags in window class
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK: {
                MouseButtonDoubleClickEvent e(mapMsgToButton(uMsg), (int16_t) lParam >> 16, (int16_t) lParam, wParam);
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN: {
                MouseButtonPressEvent e(mapMsgToButton(uMsg), (int16_t) lParam >> 16, (int16_t) lParam, wParam);
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP: {
                MouseButtonReleaseEvent e(mapMsgToButton(uMsg), (int16_t) lParam >> 16, (int16_t) lParam, wParam);
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_MOUSEHOVER: { // TODO: handle mouse enter and mouse leave, currently not working correctly
                MouseEnterEvent e;
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_MOUSELEAVE: {
                MouseLeaveEvent e;
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_MOUSEMOVE: {
                MouseMoveEvent e((int16_t) lParam >> 16, (int16_t) lParam, wParam);
                ((Engine*) TokiWindowsWindow::engine)->onEvent(e);
                break;
            }
            case WM_CLOSE: {
                ((TokiWindowsWindow*) ((Engine*) TokiWindowsWindow::engine)->getWindow().get())->shouldWindowClose = true;
            }
            default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }

}