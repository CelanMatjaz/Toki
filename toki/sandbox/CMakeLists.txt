add_executable(${SANDBOX_TARGET_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src/sandbox.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/test_layer.cpp
)

target_include_directories(${SANDBOX_TARGET_NAME}
	PRIVATE src)

target_link_libraries(${SANDBOX_TARGET_NAME}
	PRIVATE ${CORE_TARGET_NAME}
	PRIVATE ${ENGINE_TARGET_NAME}
	PRIVATE ${RENDERER_TARGET_NAME}
)

set_common_options(${SANDBOX_TARGET_NAME})
