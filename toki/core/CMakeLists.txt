add_library(${CORE_TARGET_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src/print.cpp
)

target_include_directories(${CORE_TARGET_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# target_sources(${CORE_TARGET_NAME}
	# 	PRIVATE platform/win32/platform_window_win32.cpp
	# 	PRIVATE platform/win32/platform_win32.cpp
	# 	PRIVATE platform/win32/platform_filesystem_win32.cpp
	# )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_sources(${CORE_TARGET_NAME}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_platform.cpp
	)
	if(TOKI_NO_STDLIB)
		target_sources(${CORE_TARGET_NAME}
			PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux_entry.cpp)
	endif()
else()
	message(FATAL_ERROR "Platform not supported")
endif()

set_common_options(${CORE_TARGET_NAME})
add_pch(
	${CORE_TARGET_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src/tkpch_core.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/tkpch_core.cpp
)
