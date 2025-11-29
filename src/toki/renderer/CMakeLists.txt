set(DEPS core)
add_library_target(renderer ${CMAKE_CURRENT_SOURCE_DIR} "${DEPS}")

target_link_libraries(renderer PUBLIC
	vulkan
	shaderc
	shaderc_combined
	SPIRV-Tools
	SPIRV-Tools-opt
	glslang
    glslang-default-resource-limits)

target_link_directories(renderer
	PUBLIC ${VULKAN_SDK}/lib)

target_include_directories(renderer
	PUBLIC ${VULKAN_SDK}/include)
