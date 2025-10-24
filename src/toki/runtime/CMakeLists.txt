set(DEPS core renderer)
add_library_target(runtime ${CMAKE_CURRENT_SOURCE_DIR} "${DEPS}")

target_include_directories(runtime
	PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb)
