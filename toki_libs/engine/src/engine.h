#pragma once

#include <toki/core.h>
#include <toki/renderer.h>

#include "window.h"

namespace toki {

class Engine {
public:
    struct Config;

public:
    Engine() = delete;
    Engine(const Config& config);
    ~Engine();

    DELETE_COPY(Engine);
    DELETE_MOVE(Engine);

    void run();

    void add_window(const char* title, u32 width, u32 height);

private:
    Allocator mGlobalAllocator;
    DoubleBumpAllocator mFrameAllocator;
    BasicRef<RendererFrontend, Allocator> mRenderer;
    b32 mIsRunning = false;

public:
    struct Config {
        struct {
            // Engine's total heap block allocation size
            //
            // Everything from the engine is allocated from
            // a buffer of this size
            u64 engine_memory_block_size;

            // Engine's frame allocator block size
            //
            // This value represents the size of EACH of 2
            // blocks in the engine's DoubleBlockAllocator
            // _frame_allocator member variable
            u64 engine_frame_memory_block_size;

            // Renderer's frame allocator block size
            //
            // This value represents the size of EACH of 2
            // blocks in the renderer's DoubleBlockAllocator
            // _frame_allocator member variable
            u64 renderer_frame_memory_block_size;
        } memory_config{};
    };
};

}  // namespace toki
