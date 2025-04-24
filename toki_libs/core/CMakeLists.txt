set(CORE_TARGET_NAME Core)

add_library(${CORE_TARGET_NAME} STATIC
	tkpch_core.cpp
	core/logging.cpp
)

target_include_directories(${CORE_TARGET_NAME}
	PRIVATE .
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_sources(${CORE_TARGET_NAME}
		PRIVATE platform/win32/platform_window_win32.cpp
		PRIVATE platform/win32/platform_win32.cpp
		PRIVATE platform/win32/platform_filesystem_win32.cpp
	)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_sources(${CORE_TARGET_NAME}
		PRIVATE platform/linux/linux_stream.cpp
		PRIVATE platform/linux/linux_print.cpp
		PRIVATE platform/linux/linux_filesystem.cpp
		PRIVATE platform/linux/linux_platform.cpp
		PRIVATE platform/linux/linux_wayland.cpp
		PRIVATE platform/linux/linux_socket.cpp
		PRIVATE platform/linux/linux_time.cpp
		PRIVATE platform/linux/linux_file.cpp
	)
else()
	message(FATAL_ERROR "Platform not supported")
endif()

set_common_options(${CORE_TARGET_NAME})

