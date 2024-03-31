#pragma once

#define TK_WINDOW_SYSTEM_GLFW

#if defined(WIN32) || defined(_WIN32)
#define TK_PLATFORM_WINDOWS
#include <Windows.h>
#endif