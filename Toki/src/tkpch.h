#pragma once

#include "vulkan/vulkan.h"
#include "imgui.h"

#ifdef TK_WIN32
#define _WIN32_WINNT 0x0A00
#include "Windows.h"
#include "WinUser.h"
#include "vulkan/vulkan_win32.h"
#else
#include "GLFW/glfw3.h"
#endif

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#include "algorithm"
#include "array"
#include "chrono"
#include "cstdint"
#include "filesystem"
#include "format"
#include "fstream"
#include "functional"
#include "iostream"
#include "numeric"
#include "stdexcept"
#include "unordered_map"
#include "vector"
#include "utility"