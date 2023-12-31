#pragma once

#include <condition_variable>
#include <queue>
#include <vector>

#include "core.h"
#include "core/layer.h"
#include "events/events.h"
#include "job_system/job_system.h"
#include "renderer/renderer.h"
#include "window.h"

namespace Toki {

class Application {
public:
    Application();
    ~Application();

    void run();
    void close();

    static Application& get();

    static std::future<void> queueJob(Ref<Job> job);
    static uint32_t getWorkerThreadCount();

    void onEvent(Event event);
    void pushLayer(Ref<Layer> layer);
    void popLayer();

private:
    void cleanup();

    inline static Application* app = nullptr;

    inline static Ref<Window> window;
    inline static Ref<Renderer> renderer;
    inline static Scope<JobSystem> jobSystem;

    inline static bool running = true;
    inline static bool isInitialized = false;
    inline static bool isMinimized = false;

    static void gameLoop();
    static void renderLoop();

    inline static std::shared_mutex gameLoopLock;
    inline static std::shared_mutex renderLoopLock;
    inline static float forcedFPS = 144.0f;
    inline static float deltaTime = 0;

    static void eventHandler();
    inline static std::queue<Event> eventQueue;
    inline static std::shared_mutex eventHandlerLock;
    inline static std::condition_variable_any eventHandlerCv;

    inline static std::vector<Ref<Layer>> layerStack;
};

}  // namespace Toki
