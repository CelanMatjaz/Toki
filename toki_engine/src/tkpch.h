#pragma once

#include "toki/core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

// std

#include <print>

// Libraries

#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
