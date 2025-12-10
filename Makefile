ifneq (,$(wildcard .env))
    include .env
endif

CMAKE_DEFINES := \
 	-G$(GENERATOR) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DTOKI_USE_GLFW=$(TOKI_USE_GLFW) \

UNAME := $(shell uname -n | tr '[:upper:]' '[:lower:]')

submodule:
	git submodule update --init --recursive

generate: 
	mkdir -p build/$(UNAME)
	cmake -S . -B build $(CMAKE_DEFINES) -GNinja

build-linux: generate
	cmake --build build/$(UNAME)

generate-tests:
	cmake -S . -B build $(CMAKE_DEFINES) -GNinja

build-tests: generate-tests
	cmake --build build

test: build-tests
	./build/bin/tests

docker-image:
	docker build -t toki-linux-image .

docker-build-linux: docker-image
	docker run -it --rm -v ".:/toki" toki-linux-image

docker-build:
	echo ${VULKAN_SDK}
	mkdir -p /build
	cmake -S . -B /build $(CMAKE_DEFINES)
	cmake --build /build
	cp -r /build/bin /toki/build/bin

docker-clean:
	docker rm -f toki-build-linux

clean:
	rm -r build






