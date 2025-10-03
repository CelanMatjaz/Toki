cmake_minimum_required(VERSION 3.23)
project(Toki LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("cmake/common.global.cmake")
include("cmake/common.project.cmake")

option(TOKI_USE_GLFW "Build and use GLFW submodule" OFF)

if(TOKI_GLFW_SOURCE)
	add_definitions(
		-DGLFW_INCLUDE_VULKAN
	)
	add_subdirectory(vendor/glfw)
endif()

add_subdirectory(toki)

add_compile_definitions(-fconcepts-diagnostics-depth=10)
