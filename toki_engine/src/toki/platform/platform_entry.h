#pragma once

int tk_entry_point(int argc, char** args);

#if defined(TK_PLATFORM_WINDOWS)

#include <Windows.h>

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_cmd);

#endif
