add_library(${ENGINE_TARGET_NAME} STATIC
	engine.cpp)

target_include_directories(${ENGINE_TARGET_NAME}
	PRIVATE src
	INTERFACE include)

target_link_libraries(${ENGINE_TARGET_NAME}
	PUBLIC ${CORE_TARGET_NAME}
	PUBLIC ${RENDERER_TARGET_NAME})

set_common_options(${ENGINE_TARGET_NAME})
