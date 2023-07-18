#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
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

#define TK_ASSERT(assertCondition, ...) if(assertCondition); else throw std::runtime_error(std::format("Assertion error {}:{} - {}\n", __FILE__, __LINE__, #assertCondition))
#define TK_ASSERT_VK_RESULT(vkFunction) if(vkFunction == VK_SUCCESS); else throw std::runtime_error(std::format("Assertion error {}-{}:{}", #vkFunction, __FILE__, __LINE__))