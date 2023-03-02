#include "application.h"

#include "tkpch.h"
#include "toki/renderer/vulkan/vulkan_pipeline.h"

namespace Toki {

    Application::Application() {
        glfwInit();

        TK_ASSERT(application == nullptr);
        application = this;

        window = TokiWindow::createWindow("Window", 1280, 720, true);

        TK_ASSERT(rendererBackend == nullptr);
        Application::rendererBackend = new VulkanRenderer();

        layerStack = new LayerStack();
    }

    Application::~Application() {
        delete layerStack;
        VulkanPipeline::cleanup();
        delete Application::rendererBackend;
    }

    void Application::run() {
        while (running) {
            float now = glfwGetTime();
            float deltaTime = now - lastFrameTime;
            lastFrameTime = now;

            rendererBackend->beginFrame();

            for (int layerIndex = layerStack->layers.size() - 1; layerIndex >= 0; --layerIndex)
                layerStack->layers[layerIndex]->onUpdate(deltaTime);

            rendererBackend->endFrame();

            glfwPollEvents();
            if (glfwWindowShouldClose(window->getHandle())) running = false;
        }
    }

    void Application::addLayer(Layer* layer) {
        layerStack->pushLayer(layer);
    }

    void Application::onEvent(Event& event) {
        switch (event.getType()) {
            // TODO: add events
        }
    }

}