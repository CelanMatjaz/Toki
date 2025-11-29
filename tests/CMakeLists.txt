cmake_minimum_required(VERSION 3.23)
project(Toki-testing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/**test_*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/**test_*.h
)

list(APPEND SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/testing.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/testing.h)

set(DEPS core)

add_executable_target(tests ${CMAKE_CURRENT_SOURCE_DIR} "${DEPS}")

target_compile_definitions(tests PRIVATE TK_TESTING_ENABLED)
