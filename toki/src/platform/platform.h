#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS
#ifndef _WIN64
#error "64-bit OS is required"
#endif
// TODO: expand for other platforms
#endif

#define WINDOW_SYSTEM_NATIVE
// #define WINDOW_SYSTEM_GLFW

#ifdef PLATFORM_WINDOWS

#endif
