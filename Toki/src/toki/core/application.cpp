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
        delete  Application::rendererBackend;
    }

    void Application::run() {
        while (running) {
            float now = glfwGetTime();
            float deltaTime = now - lastFrameTime;
            lastFrameTime = now;

            rendererBackend->beginFrame();
            onUpdate(deltaTime);
            rendererBackend->endFrame();

            glfwPollEvents();
            if (glfwWindowShouldClose(window->getHandle())) running = false;
        }
    }

}