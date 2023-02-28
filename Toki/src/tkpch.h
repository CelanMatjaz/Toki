#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#include "algorithm"
#include "array"
#include "chrono"
#include "filesystem"
#include "fstream"
#include "numeric"
#include "iostream"
#include "stdexcept"
#include "vector"

// TODO: move later
#define ENGINE_NAME "Toki"
#define ENGINE_VERSION 1

#define TK_ASSERT(arg) if(arg); else throw std::runtime_error(std::string{ "Assertion error "} + std::string{ __FILE__ } + std::string{ ":" } + std::string{ std::to_string(__LINE__) } + std::string { "\n   " } /* + std::string { #__VA_ARGS__  } */);