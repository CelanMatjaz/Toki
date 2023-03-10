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
        imGuiLayer = new ImGuiLayer();
        imGuiLayer->onAttach();
    }

    Application::~Application() {
        delete layerStack;
        imGuiLayer->onDetach();
        delete imGuiLayer;
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

            { // ImGui rendering
                imGuiLayer->startImGuiFrame();
                imGuiLayer->onUpdate(deltaTime);
                for (int layerIndex = layerStack->layers.size() - 1; layerIndex >= 0; --layerIndex)
                    layerStack->layers[layerIndex]->onImGuiUpdate(deltaTime);
                imGuiLayer->endImGuiFrame();
            }

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

        imGuiLayer->onEvent(event);

        for (int i = layerStack->layers.size() - 1; i >= 0 && event.isHandled(); --i) {
            layerStack->layers[i]->onEvent(event);
            if (event.isHandled()) break;
        }
    }

}