#include "toki_entry.h"

#include <toki/core.h>

#include "window.h"

#if defined(TK_PLATFORM_WINDOWS)
namespace toki {
extern LRESULT window_proc(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param);
}
#endif

#if defined(TK_DIST) && defined(TK_PLATFORM_WINDOWS)
int APIENTRY WinMain(
    HINSTANCE instance,
    [[maybe_unused]] HINSTANCE prev_instance,
    [[maybe_unused]] LPSTR command_line,
    [[maybe_unused]] int nShowCmd) {
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char** argv) {
#endif

#if defined(TK_PLATFORM_WINDOWS)
    HINSTANCE instance{};
    toki::WindowSystemInit window_init{};
    window_init.instance = instance;
    window_init.window_proc = toki::window_proc;

    toki::window_system_initialize(window_init);
#else
    toki::window_system_initialize();
#endif

    auto result = tk_entry_point(argc, argv);
    toki::window_system_shutdown();
    return result;
}
