set(DEPS core platform renderer)
add_library_target(runtime ${CMAKE_CURRENT_SOURCE_DIR} "${DEPS}")
