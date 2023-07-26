VERT_SHADERS_RAW = $(shell find ./shaders/raw -type f -name "*.vert")
VERT_SHADERS_COMPILED = $(patsubst ./shaders/raw/%.vert, ./shaders/compiled/%.vert.spv, $(VERT_SHADERS_RAW))
FRAG_SHADERS_RAW = $(shell find ./shaders/raw -type f -name "*.frag")
FRAG_SHADERS_COMPILED = $(patsubst ./shaders/raw/%.frag, ./shaders/compiled/%.frag.spv, $(FRAG_SHADERS_RAW))

all: compiled_folder $(VERT_SHADERS_COMPILED) $(FRAG_SHADERS_COMPILED)

compiled_folder:
	mkdir -p ./shaders/compiled

shaders/compiled/%.spv: ./shaders/raw/%
	${VULKAN_SDK}/Bin/glslc.exe $< -o $@