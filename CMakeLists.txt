cmake_minimum_required(VERSION 3.23)
project(Toki LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("cmake/common.env.cmake")
include("cmake/common.global.cmake")
include("cmake/common.project.cmake")

option(TOKI_USE_GLFW "Build and use GLFW submodule" OFF)
option(TOKI_ENABLE_TESTING "Build tests" OFF)

add_subdirectory(src)

if(TOKI_ENABLE_TESTING)
	add_subdirectory(tests)
endif()
