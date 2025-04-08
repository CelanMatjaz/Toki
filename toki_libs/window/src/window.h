#pragma once

#include <toki/core.h>

namespace toki {

struct WindowInitFlags {
    b8 show_on_create : 1;
};

class WindowSystem;

class Window {
    friend WindowSystem;

private:
    Window() = default;
    Window(const char* title, u32 width, u32 height, const WindowInitFlags& flags = {});

private:
    NativeHandle _handle;
};

class WindowSystem {
public:
    static void initialize();
    static void shutdown();

    static void poll_events();

    static Window* create_window(const char* title, u32 width, u32 height, const WindowInitFlags& flags = {});
    static void destroy_window(Window* window);

private:
    inline static Window s_windows[4]{};
};

}  // namespace toki
