#pragma once

#include "toki/core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <windows.h>
#endif

// std

#include <array>
#include <bitset>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <queue>
#include <set>
#include <span>
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

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
