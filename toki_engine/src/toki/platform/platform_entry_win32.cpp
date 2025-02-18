#include <print>

#include "core/assert.h"
#include "platform_entry.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Windows.h>
#include <shellapi.h>

#include "core/logging.h"

namespace toki {

namespace platform {

extern LRESULT CALLBACK window_proc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE global_instance;
extern const char* global_window_class;

}  // namespace platform

}  // namespace toki

extern int tk_entry_point(int, char**);

int WinMain(
    HINSTANCE instance,
    [[maybe_unused]] HINSTANCE prev_instance,
    [[maybe_unused]] LPSTR command_line,
    [[maybe_unused]] int show_cmd) {
    using namespace toki::platform;

#ifndef DIST
    // Display console in non dist mode
    TK_ASSERT(AllocConsole(), "Console was not opened");

    FILE* f{};
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
#endif

    global_instance = instance;

    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC | CS_HREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = global_instance;
    window_class.lpszClassName = global_window_class;

    if (!RegisterClassA(&window_class)) {
        TK_LOG_FATAL("Could not create window class");
        exit(-1);
    }

    int argc = 0;
    char** argv = nullptr;

    return tk_entry_point(argc, argv);
}

#endif
