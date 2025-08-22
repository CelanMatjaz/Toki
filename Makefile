all: generate build

submodule:
	git submodule update --init --recursive

generate-linux: 
	mkdir -p build
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DTOKI_USE_GLFW=ON -GNinja

build-linux: generate-linux
	cmake --build build

clean:
	rm -r build
