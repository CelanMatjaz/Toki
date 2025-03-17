#include "engine.h"

#include <toki/core.h>

#include "window.h"

namespace toki {

Engine::Engine(const Config& config):
    mGlobalAllocator(config.memory_config.engine_memory_block_size),
    mFrameAllocator(mGlobalAllocator, config.memory_config.engine_frame_memory_block_size) {
    mRenderer = move(BasicRef<RendererFrontend>(mGlobalAllocator));
}

Engine::~Engine() {
    for (u32 i = 0; i < sWindowCount; i++) {
        window_destroy(sWindows[i].mNativeHandle);
    }
}

void Engine::run() {
    mIsRunning = true;

    static auto last_frame_time = time_microseconds();
    [[maybe_unused]] float delta_time = 0;

    EventHandler h;

    while (mIsRunning) {
        for (u32 i = 0; i < sWindowCount; i++) {
            sWindows[i].handle_events(h);
        }

        auto frame_start_time = time_microseconds();
        delta_time = (frame_start_time - last_frame_time) / 1'000'000.0f;
        last_frame_time = frame_start_time;

        mFrameAllocator.swap();
        mFrameAllocator->clear();
    }
}

void Engine::add_window(const char* title, u32 width, u32 height) {
    TK_ASSERT(sWindowCount < MAX_ENGINE_WINDOW_COUNT, "Adding another window would go over current limit");

    auto handle = window_create(title, width, height, WindowInitFlags{ .show_on_create = true });
    sWindows[sWindowCount++].mNativeHandle = handle;
}

}  // namespace toki
