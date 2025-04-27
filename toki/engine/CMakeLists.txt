set(ENGINE_TARGET_NAME Engine)

add_library(${ENGINE_TARGET_NAME} STATIC
	engine.cpp
)

target_include_directories(${ENGINE_TARGET_NAME}
	PRIVATE src
)

target_link_libraries(${ENGINE_TARGET_NAME} ${CORE_TARGET_NAME})

set_common_options(${ENGINE_TARGET_NAME})
