cmake_minimum_required(VERSION 3.16...3.28)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOKI_USE_GLFW "Build and use GLFW submodule" OFF)

# Make debug default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    message("No build configuration specified... Using Debug.")
endif()

add_compile_definitions(
    $<$<CONFIG:Debug>:TK_DEBUG>
    $<$<CONFIG:Release>:TK_NDEBUG>
    $<$<CONFIG:Release>:TK_RELEASE>
    $<$<CONFIG:Dist>:TK_NDEBUG>
    $<$<CONFIG:Dist>:TK_DIST>
)

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

function(set_common_options target)
    message("Target: ${target}")

    target_link_options(${target} PRIVATE -nostdlib -nostartfiles -fno-stack-protector)

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

function(add_pch target pch pch_source)
    target_precompile_headers(${target} PRIVATE ${pch})

    if(MSVC)
        target_sources(${target} PRIVATE ${pch_source})
        target_compile_options(${target} -/FI${pch})
    endif()
endfunction()

add_subdirectory(toki)

if(TOKI_GLFW_SOURCE)
    add_subdirectory(vendor/glfw)
endif()
