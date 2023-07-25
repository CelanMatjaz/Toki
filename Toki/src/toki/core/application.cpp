#include "tkpch.h"
#include "application.h"

#include "tkpch.h"
#include "toki/renderer/vulkan/vulkan_pipeline.h"

namespace Toki {

    Application::Application(RendererAPI::API api) {
        glfwInit();

        TK_ASSERT(application == nullptr);
        application = this;

        window = TokiWindow::createWindow("Window", 1280, 720, true);

        TK_ASSERT(rendererBackend == nullptr);
        Application::rendererApi = api;
        switch (api) {
            case RendererAPI::API::VULKAN:
                Application::rendererBackend = new VulkanRenderer();
                break;
            case RendererAPI::API::NONE:
            default:
                TK_ASSERT(false && "No renderer api provided");
        }

        rendererBackend->init();

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
        glfwTerminate();
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
        imGuiLayer->onEvent(event);

        if (event.isHandled()) return;

        for (int i = layerStack->layers.size() - 1; i >= 0 && !event.isHandled(); --i) {
            layerStack->layers[i]->onEvent(event);
        }
    }

}