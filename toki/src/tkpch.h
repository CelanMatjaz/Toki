#pragma once

#ifndef UNICODE
#define UNICODE
#endif

// Standard lib
#include "algorithm"
#include "atomic"
#include "chrono"
#include "filesystem"
#include "fstream"
#include "future"
#include "iostream"
#include "memory"
#include "mutex"
#include "queue"
#include "shared_mutex"
#include "string"
#include "thread"
#include "unordered_map"
#include "vector"

// Platform macros

#include "platform/platform.h"

#if !defined(WINDOW_SYSTEM_NATIVE) && !defined(WINDOW_SYSTEM_GLFW)
#error "Window system to be used not defined"
#endif

#ifdef WINDOW_SYSTEM_NATIVE

#ifdef PLATFORM_WINDOWS
#define WINDOW_SYSTEM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include "windows.h"
#include "winuser.h"
#endif

#ifdef PLATFORM_LINUX
// TODO
#endif

#endif

#ifdef WINDOW_SYSTEM_GLFW
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#endif

// 3rd party

#include "imgui.h"
#include "vulkan/vulkan.h"
