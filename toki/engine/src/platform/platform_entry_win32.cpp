#include "platform_entry.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Windows.h>
#include <shellapi.h>

namespace toki {

extern LRESULT CALLBACK window_proc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE global_instance;
extern const char* global_window_class;

}  // namespace toki

extern int tk_entry_point(int, char**);

#if defined(TK_DIST)
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_cmd) {
#ifndef DIST
    // Display console in non dist mode
    TK_ASSERT(AllocConsole(), "Console was not opened");

    FILE* f{};
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
#endif


    int argc = 0;
    char** argv = nullptr;
#else
int main(int argc, char** argv) {
#endif

    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC | CS_HREDRAW;
    window_class.lpfnWndProc = DefWindowProc;
    window_class.hInstance = 0;
    window_class.lpszClassName = global_window_class;

    if (!RegisterClassA(&window_class)) {
        TK_LOG_FATAL("Could not create window class");
        exit(-1);
    }

    return tk_entry_point(argc, argv);
}

#endif
