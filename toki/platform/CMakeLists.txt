add_library_target(${PLATFORM_TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PLATFORM_TARGET_NAME} PRIVATE ${CORE_TARGET_NAME})

if(TOKI_USE_GLFW)
    # target_link_libraries(${PLATFORM_TARGET_NAME} glfw)
    # target_sources(${CORE_TARGET_NAME}
    #     PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/glfw_window.cpp)
endif()

# if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
# elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     target_sources(${CORE_TARGET_NAME}
#         PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_platform.cpp)
#     if(NOT TOKI_USE_GLFW)
#         target_sources(${CORE_TARGET_NAME}
#             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_wayland.cpp)
#     endif()
#
#     if(TOKI_NO_STDLIB)
#         target_sources(${CORE_TARGET_NAME}
#             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_entry.cpp)
#     endif()
# else()
#     message(FATAL_ERROR "Platform not supported")
# endif()
#
# add_pch(
#     ${CORE_TARGET_NAME}
#     ${CMAKE_CURRENT_SOURCE_DIR}/tkpch_core.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/tkpch_core.cpp
# )
