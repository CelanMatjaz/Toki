#include "application.h"

#include "tkpch.h"

namespace Toki {

    Application::Application() {
        glfwInit();

        TK_ASSERT(application == nullptr);
        application = this;

        window = TokiWindow::createWindow("Window", 1280, 720, true);

        TK_ASSERT(rendererBackend == nullptr);
        Application::rendererBackend = new VulkanRenderer();
    }

    Application::~Application() {
        delete Application::rendererBackend;
    }

    void Application::run() {
        const auto& [windowWidth, windowHeight] = window->getWindowDimensions();

        while (running) {
            glfwPollEvents();
            if (glfwWindowShouldClose(window->getHandle())) running = false;

            const auto& [windowWidth, windowHeight] = window->getWindowDimensions();
            if (windowWidth <= 0 || windowHeight <= 0) continue;

            float now = glfwGetTime();
            float deltaTime = now - lastFrameTime;
            lastFrameTime = now;

            rendererBackend->beginFrame();
            onUpdate(deltaTime);
            rendererBackend->endFrame();
        }
    }

}