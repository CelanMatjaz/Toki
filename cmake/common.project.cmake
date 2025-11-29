set(COMMON_WARNINGS
	-Wall
	-Wextra
	-Wpedantic
	-Wshadow
	-Wundef
	-Wnull-dereference
	-Wuninitialized
	-Wmissing-field-initializers
)

function(common_target_options target dir deps)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"

		OBJECT_DIR "${CMAKE_BINARY_DIR}/${target}"
	)

	set_target_properties(${target} PROPERTIES
		OBJECT_OUTPUTS "${CMAKE_BINARY_DIR}/obj"
	)

	target_link_libraries(${target} PUBLIC ${deps})

	file(GLOB_RECURSE SOURCES
		${dir}/**.cpp
		${dir}/**.h
	)

	# list(FILTER SOURCES EXCLUDE REGEX "${dir}/platform/.*")

	# message(${dir})
	# foreach(var ${SOURCES})
	# 	message(${var})
	# endforeach()

	target_sources(${target}
		PRIVATE ${SOURCES}
	)

	target_include_directories(${target}
		PUBLIC ${CMAKE_SOURCE_DIR}/src
	)

	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:TK_DEBUG>
		$<$<CONFIG:Release>:TK_NDEBUG>
		$<$<CONFIG:Release>:TK_RELEASE>
		$<$<CONFIG:Dist>:TK_NDEBUG>
		$<$<CONFIG:Dist>:TK_DIST>
		${TOKI_WINDOW_SYSTEM}
	)

	# Include platform specific files
	if(EXISTS "${dir}/platform")
		if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
			set(platform_target "win32")
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			set(platform_target "linux")
		else()
			message(FATAL_ERROR "Invalid target system " ${CMAKE_SYSTEM_NAME})
		endif()

		file(GLOB_RECURSE PLATFORM_SOURCES
			${dir}/${platform_target}/**.cpp
			${dir}/${platform_target}/**.h
		)
		target_sources(${target} PRIVATE ${PLATFORM_SOURCES})
	endif()

	if(MSVC)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
					CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(${target}
				PRIVATE -ftrivial-auto-var-init=pattern -fno-exceptions ${COMMON_WARNINGS})
	endif()

	if(TOKI_USE_GLFW)
		target_link_libraries(${target} PUBLIC glfw)
	endif()

	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIR}"
		LIBRARY_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIR}"
		ARCHIVE_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIR}"
	)
endfunction()

function(add_library_target target dir deps)
	add_library(${target} STATIC)
	common_target_options(${target} ${dir} "${deps}")
endfunction()

function(add_executable_target target dir deps)
	add_executable(${target})
	common_target_options(${target} ${dir} "${deps}")
endfunction()

function(add_pch target pch pch_source)
	target_precompile_headers(${target} PRIVATE ${pch})

	# MSVC hack to include pch header in every translation unit
	if(MSVC)
		target_sources(${target} PRIVATE ${pch_source})
		target_compile_options(${target} -/FI${pch})
	endif()
endfunction()
