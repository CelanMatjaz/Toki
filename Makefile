ifneq (,$(wildcard .env))
    include .env
endif

CMAKE_DEFINES := \
 	-G$(GENERATOR) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DTOKI_USE_GLFW=$(TOKI_USE_GLFW) \

UNAME := $(shell uname -n | tr '[:upper:]' '[:lower:]')

OUTPUT_DIR := build/local

submodule:
	git submodule update --init --recursive

generate: 
	mkdir -p $(OUTPUT_DIR)
	cmake -S . -B $(OUTPUT_DIR) $(CMAKE_DEFINES) -GNinja

build-linux: generate
	cmake --build $(OUTPUT_DIR)

generate-tests:
	cmake -S . -B $(OUTPUT_DIR) $(CMAKE_DEFINES) -DTOKI_ENABLE_TESTING=ON -GNinja

build-tests: generate-tests
	cmake --build $(OUTPUT_DIR) --target tests

test: build-tests
	./$(OUTPUT_DIR)/bin/tests

docker-clean:
	docker rm -f toki-build-linux

clean:
	rm -r build






