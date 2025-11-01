# Setup Vulkan
if(NOT DEFINED ENV{VULKAN_SDK})
	message(FATAL " No VULKAN_SDK env variable found")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}")
endforeach()

set(VULKAN_SDK $ENV{VULKAN_SDK})
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

# Setup gcc and clang color outputs
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fdiagnostics-color=always)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-fcolor-diagnostics)
endif()

# Make debug default
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
	message(STATUS "No build configuration specified. Defaulting to Debug.")
endif()

message(STATUS "Using configuration: " ${CMAKE_BUILD_TYPE})

# Setup platform specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_compile_definitions(TK_PLATFORM_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_compile_definitions(TK_PLATFORM_LINUX)
else()
	message(FATAL_ERROR "Platform not supported")
endif()

if(TOKI_USE_GLFW)
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_GLFW)
	add_subdirectory(${VENDOR_DIR}/glfw)
else()
	set(TOKI_WINDOW_SYSTEM TK_WINDOW_SYSTEM_NATIVE)
endif()

