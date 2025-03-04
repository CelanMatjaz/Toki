#include "toki_entry.h"

#include <toki/core.h>

#include "window.h"

#if defined(TK_PLATFORM_WINDOWS)

#if defined(TK_DIST)
int APIENTRY WinMain(
    HINSTANCE instance,
    [[maybe_unused]] HINSTANCE prev_instance,
    [[maybe_unused]] LPSTR command_line,
    [[maybe_unused]] int nShowCmd) {
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char** argv) {
    HINSTANCE instance{};
#endif

    toki::platform::init_win32(instance, toki::toki_window_proc);
#endif

    return tk_entry_point(argc, argv);
}
