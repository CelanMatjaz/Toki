cmake_minimum_required(VERSION 3.23)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOKI_USE_GLFW "Build and use GLFW submodule" OFF)

if(NOT DEFINED ENV{VULKAN_SDK})
	message(FATAL "No VULKAN_SDK env variable found")
endif()

set(VULKAN_SDK $ENV{VULKAN_SDK})
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fdiagnostics-color=always)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-fcolor-diagnostics)
endif()

# Make debug default
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
	message("No build configuration specified... Using Debug.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_compile_definitions(TK_PLATFORM_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_compile_definitions(TK_PLATFORM_LINUX)

	# set(XDG_SESSION_TYPE $ENV{XDG_SESSION_TYPE})
	# if(XDG_SESSION_TYPE STREQUAL "wayland")
	# elseif(XDG_SESSION_TYPE STREQUAL "x11")
	# endif()

else()
	message(FATAL_ERROR "Platform not supported")
endif()

if(TOKI_USE_GLFW)
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_GLFW)
	add_subdirectory(${VENDOR_DIR}/glfw)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_WINDOWS)
elseif(DEFINED ENV{WAYLAND_DISPLAY})
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_WAYLAND)
elseif(DEFINED ENV{DISPLAY})
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_X11)
elseif(DEFINED ENV{XDG_SESSION_TYPE})
	if("$ENV{XDG_SESSION_TYPE}" STREQUAL "wayland")
		set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_WAYLAND)
	elseif("$ENV{XDG_SESSION_TYPE}" STREQUAL "x11")
		set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_X11)
	else()
		message("Unknown session type: XDG_SESSION_TYPE=$ENV{XDG_SESSION_TYPE}")
	endif()
else()
	message(STATUS "No supported window system detected")
endif()

function(set_common_options target)
	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:TK_DEBUG>
		$<$<CONFIG:Release>:TK_NDEBUG>
		$<$<CONFIG:Release>:TK_RELEASE>
		$<$<CONFIG:Dist>:TK_NDEBUG>
		$<$<CONFIG:Dist>:TK_DIST>
		${TOKI_WINDOW_SYSTEM}
		TK_MAX_WINDOW_COUNT=1
	)

	if(MSVC)
	elseif(
			CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
			CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(${target}
				PRIVATE -ftrivial-auto-var-init=pattern -fno-exceptions -Wuninitialized)
	endif()
endfunction()

function(add_pch target pch pch_source)
	target_precompile_headers(${target} PRIVATE ${pch})

	# MSVC hack to include header in every translation unit
	if(MSVC)
		target_sources(${target} PRIVATE ${pch_source})
		target_compile_options(${target} -/FI${pch})
	endif()
endfunction()

add_subdirectory(toki)

if(TOKI_GLFW_SOURCE)
	add_subdirectory(vendor/glfw)
endif()
