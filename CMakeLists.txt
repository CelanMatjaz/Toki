cmake_minimum_required(VERSION 3.16...3.28)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOKI_GLFW_SOURCE "Build and link against GLFW submodule" OFF)

function(set_common_options target)
	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:TK_DEBUG>
		$<$<CONFIG:Release>:TK_NDEBUG>
		$<$<CONFIG:Release>:TK_RELEASE>
		$<$<CONFIG:Dist>:TK_NDEBUG>
		$<$<CONFIG:Dist>:TK_DIST>
	)

	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		target_compile_definitions(${target} PRIVATE TK_PLATFORM_WINDOWS)
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_compile_definitions(${target} PRIVATE TK_PLATFORM_LINUX)

		# set(XDG_SESSION_TYPE $ENV{XDG_SESSION_TYPE})
		# if(XDG_SESSION_TYPE STREQUAL "wayland")
		# elseif(XDG_SESSION_TYPE STREQUAL "x11")
		# endif()

	else()
		message(FATAL_ERROR "Platform not supported")
	endif()

	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		message(STATUS "Using GNU compiler")

	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message(STATUS "Using Clang compiler")
		# Clang-specific flags or defines
		target_compile_definitions(${target} PRIVATE MYAPP_CLANG)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		message(STATUS "Using MSVC compiler")
		# MSVC-specific flags or defines
		target_compile_definitions(${target} PRIVATE MYAPP_MSVC)
	else()
		message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
	endif()

endfunction()

add_subdirectory(toki_libs)

if(TOKI_GLFW_SOURCE)
	add_subdirectory(vendor/glfw)
endif()
