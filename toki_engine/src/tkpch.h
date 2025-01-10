#pragma once

#include "toki/core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#define NOMINMAX
#include <windows.h>
#endif

// std

#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <queue>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// Libraries

#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
