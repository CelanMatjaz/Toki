add_library(${RENDERER_TARGET_NAME} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/vulkan_backend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/vulkan_commands.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/renderer_frontend.cpp
)

target_include_directories(${RENDERER_TARGET_NAME}
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/toki
	PRIVATE ${VULKAN_SDK}/include)
target_link_libraries(${RENDERER_TARGET_NAME}
	PRIVATE ${CORE_TARGET_NAME} vulkan shaderc_shared)
target_link_directories(${RENDERER_TARGET_NAME}
	PRIVATE ${VULKAN_SDK}/lib)

set_common_options(${RENDERER_TARGET_NAME})

if(TOKI_USE_GLFW)
	target_link_directories(${RENDERER_TARGET_NAME}
		PRIVATE ${VENDOR_DIR})
	target_link_libraries(${RENDERER_TARGET_NAME}
		PRIVATE glfw)
	target_compile_definitions(${RENDERER_TARGET_NAME}
		PRIVATE GLFW_INCLUDE_VULKAN)
	target_compile_definitions(glfw
		PRIVATE GLFW_VULKAN_STATIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_sources(${RENDERER_TARGET_NAME}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/vulkan_platform_win32.cpp
	)

	target_compile_definitions(${RENDERER_TARGET_NAME}
		PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_sources(${RENDERER_TARGET_NAME}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/vulkan_platform_linux.cpp
	)

	if(TOKI_WINDOW_SYSTEM STREQUAL "TK_WINDOW_SYSTEM_WAYLAND")
		target_compile_definitions(${RENDERER_TARGET_NAME}
		PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
	elseif(TOKI_WINDOW_SYSTEM STREQUAL "TK_WINDOW_SYSTEM_X11")
		target_compile_definitions(${RENDERER_TARGET_NAME}
		PRIVATE VK_USE_PLATFORM_X11_KHR)
	endif()
else()
	message(FATAL_ERROR "Platform not supported")
endif()

