set(CORE_TARGET_NAME Core)

add_library(${CORE_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/print.cpp
)

target_include_directories(${CORE_TARGET_NAME}
    PUBLIC src
    INTERFACE include
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # target_sources(${CORE_TARGET_NAME}
    # 	PRIVATE platform/win32/platform_window_win32.cpp
    # 	PRIVATE platform/win32/platform_win32.cpp
    # 	PRIVATE platform/win32/platform_filesystem_win32.cpp
    # )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(${CORE_TARGET_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_entry.cpp
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_platform.cpp
 )
else()
    message(FATAL_ERROR "Platform not supported")
endif()

set_common_options(${CORE_TARGET_NAME})
add_pch(
    ${CORE_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tkpch_core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tkpch_core.cpp)
