function(common_target_options target dir)
	target_include_directories(${target}
		PRIVATE ${dir}/src
		INTERFACE ${dir}/include
	)

	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:TK_DEBUG>
		$<$<CONFIG:Release>:TK_NDEBUG>
		$<$<CONFIG:Release>:TK_RELEASE>
		$<$<CONFIG:Dist>:TK_NDEBUG>
		$<$<CONFIG:Dist>:TK_DIST>
		${TOKI_WINDOW_SYSTEM}
	)

	if(MSVC)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
					CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(${target}
				PRIVATE -ftrivial-auto-var-init=pattern -fno-exceptions -Wuninitialized)
	endif()

endfunction()

function(add_library_target target dir)
	file(GLOB_RECURSE SOURCES
		${dir}/**.cpp
		${dir}/src/**.cpp
		${dir}/include/**.h
	)

	list(FILTER SOURCES EXCLUDE REGEX ".*/src/platform/(win32|linux)/.*")

	function(add_platform_sources target dir)
		file(GLOB_RECURSE PLATFORM_SOURCES
			${dir}/src/platform/**.cpp
			${dir}/include/toki/${target}/platform/**.h
		)
		target_sources(${target} PRIVATE ${PLATFORM_SOURCES})

	endfunction()

	add_library(${target} STATIC)

	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		add_platform_sources(${target} ${dir})
	endif()

	target_sources(${target}
		PRIVATE ${SOURCES}
	)

	target_include_directories(${target}
		PUBLIC ${dir}/include
	)

	common_target_options(${target} dir)

		# foreach(s ${SOURCES})
		# 	message(source ${s})
		# 	endforeach()

endfunction()

function(add_executable_target target dir)
	file(GLOB_RECURSE SOURCES
		${dir}/src/*.cpp
	)

	add_executable(${target}
		${SOURCES})

	common_target_options(${target} dir)

endfunction()

function(add_pch target pch pch_source)
	target_precompile_headers(${target} PRIVATE ${pch})

	# MSVC hack to include header in every translation unit
	if(MSVC)
		target_sources(${target} PRIVATE ${pch_source})
		target_compile_options(${target} -/FI${pch})
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
