#include "application.h"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <cwchar>
#include <mutex>
#include <ostream>
#include <shared_mutex>

#include "GLFW/glfw3.h"
#include "chrono"
#include "core/application.h"
#include "core/assert.h"
#include "core/core.h"
#include "events/events.h"
#include "filesystem"
#include "glfw/glfw_window.h"
#include "iostream"
#include "job_system/job.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "resources/loaders/shader_loader.h"
#include "thread"

#if defined(WINDOW_SYSTEM_WINDOWS)
#include "platform/windows/windows_window.h"
#elif defined(WINDOW_SYSTEM_GLFW)
#include "platform/glfw/glfw_window.h"
#endif

namespace Toki {

struct Timer {
    Timer() { start = std::chrono::high_resolution_clock::now(); }
    ~Timer() {
        auto duration = (std::chrono::duration(std::chrono::high_resolution_clock::now() - start));
        std::cout << duration << '\n';
    }

    std::chrono::steady_clock::time_point start;
};

static void loadShader() {
    std::cout << "Current path: " << std::filesystem::current_path() << '\n';
    {
        Timer t;
        ShaderLoader::loadShader("assets/shaders/test_shader.glsl");
    }
}

Application::Application() {
    TK_ASSERT(app == nullptr, "Application already created");
    Application::app = this;
    isInitialized = true;

    WindowConfig windowConfig{};
    windowConfig.title = L"時 Window";
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.resizable = true;

#if defined(WINDOW_SYSTEM_WINDOWS)
    window = createRef<WindowsWindow>(windowConfig);
#elif defined(WINDOW_SYSTEM_GLFW)
    window = createRef<GlfwWindow>(windowConfig);
#endif

    jobSystem = createScope<JobSystem>(4);

    auto vulkanRenderer = createRef<VulkanRenderer>(window);
    renderer = createRef<VulkanRenderer>(window);

    renderer->init();
}

Application::~Application() {}

Application& Application::get() {
    return *app;
}

std::future<void> Application::queueJob(Ref<Job> job) {
    return jobSystem->queueJob(job);
}

uint32_t Application::getWorkerThreadCount() {
    return jobSystem->getWorkerThreadCount();
}

void Application::run() {
    renderLoopLock.lock();
    eventHandlerLock.lock();

    std::thread renderLoopThread(renderLoop);
    std::thread eventHandlerThread(eventHandler);

    renderLoopLock.unlock();
    eventHandlerLock.unlock();

    while (running) {
        window->pollEvents();
        if (window->shouldClose()) {
            break;
        }

    }

    renderLoopThread.join();
    eventHandlerThread.join();
}

void Application::close() {
    {
        std::scoped_lock rlck(renderLoopLock);
        std::scoped_lock ehlck(eventHandlerLock);
        running = false;
        cleanup();
    }

    window->close();
}

void Application::cleanup() {
    const uint32_t layerCount = layerStack.size();
    for (uint32_t i = 0; i < layerCount; ++i) {
        popLayer();
    }

    if (renderer) {
        renderer->shutdown();
        renderer.reset();
    }
}

void Application::onEvent(Event event) {
    switch (event.getType()) {
        case Toki::EventType::WindowMinimize:
            isMinimized = true;
            break;
        case Toki::EventType::WindowMaximize:
        case Toki::EventType::WindowResize:
            if (event.getData(0) > 0 && event.getData(1) > 0) isMinimized = false;
            break;
        case Toki::EventType::WindowClose:
            close();
            return;
        default:;
    }

    if (renderer) {
        std::scoped_lock lck(renderLoopLock);
        renderer->onEvent(event);
    }

    {
        std::scoped_lock lck(eventHandlerLock);
        eventQueue.push(event);
        eventHandlerCv.notify_all();
    }
}

void Application::pushLayer(Ref<Layer> layer) {
    layerStack.emplace_back(layer);
    layerStack.back()->onAttach();
}

void Application::popLayer() {
    layerStack.back()->onDetach();
    layerStack.erase(layerStack.end() - 1);
}

void Application::gameLoop() {
    using namespace std::literals::chrono_literals;

    return;
    while (running) {
        std::scoped_lock glck(gameLoopLock);
    }
}

void Application::renderLoop() {
    using namespace std::literals::chrono_literals;
    using namespace std::chrono;

    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    static auto msPerFrame = std::chrono::duration<float, std::milli>(1000.0f / forcedFPS);

    while (running) {
        auto frameStartTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(frameStartTime - lastFrameTime).count();
        lastFrameTime = frameStartTime;

        auto frameTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - frameStartTime);
        if (frameTime < msPerFrame) {
            std::this_thread::sleep_for(msPerFrame - frameTime);
        };

        std::scoped_lock lck(renderLoopLock);
        if (!isMinimized && running) {
            if (renderer->beginFrame()) {
                renderer->endFrame();
            }
        }
    }
}

void Application::eventHandler() {
    while (running) {
        std::shared_lock<std::shared_mutex> hlck(eventHandlerLock);
        eventHandlerCv.wait(hlck);

        while (!eventQueue.empty()) {
            Event e = eventQueue.front();
            for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
                (*it)->onEvent(e);
                if (e.getIsHandled()) break;
            }
            eventQueue.pop();
        }
    }
}

}  // namespace Toki
