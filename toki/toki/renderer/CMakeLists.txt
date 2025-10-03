set(DEPS
	${CORE_TARGET_NAME}
	${RUNTIME_TARGET_NAME}
	${PLATFORM_TARGET_NAME})
add_library_target(${RENDERER_TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR} "${DEPS}")

target_link_libraries(${RENDERER_TARGET_NAME}
	PUBLIC vulkan shaderc shaderc_combined)

target_link_directories(${RENDERER_TARGET_NAME}
	PUBLIC ${VULKAN_SDK}/lib)
