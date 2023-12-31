#pragma once

#include "core/window.h"
#include "tkpch.h"

#ifdef WINDOW_SYSTEM_WINDOWS

namespace Toki {

class WindowsWindow : public Window {
public:
    WindowsWindow() = delete;
    WindowsWindow(const WindowConfig& config);
    virtual ~WindowsWindow();

    virtual bool shouldClose() const override;
    virtual void pollEvents()  override;
    virtual void close() override;

    virtual WindowDimensions getDimensions() const override;
    virtual void* getHandle() const override;

private:
    HWND handle;
    bool shouldWindowClose = false;
    int width = 0, height = 0;

    inline static bool isWindowClassRegistered = false;
    inline static wchar_t windowClassName[] = L"Toki window class";

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}  // namespace Toki

#endif
