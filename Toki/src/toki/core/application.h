#pragma once

#include "tkpch.h"
#include "window.h"
#include "layer_stack.h"
#include "toki/events/events.h"
#include "toki/renderer/renderer_api.h"
#include "toki/renderer/vulkan/vulkan_renderer.h"
#include "toki/renderer/imgui/imgui_layer.h"

namespace Toki {
    class Application {
    public:
        Application(RendererAPI::API api = RendererAPI::API::NONE);
        virtual ~Application();

        void run();
        void addLayer(Layer* layer);

        void onEvent(Event& event);

        static GLFWwindow* getNativeWindow() { return window->getHandle(); }
        static const std::unique_ptr<TokiWindow>& getWindow() { return window; }

        static Application* getApplication() { return application; }
        static VulkanRenderer* getVulkanRenderer() {
            TK_ASSERT(rendererApi == RendererAPI::API::VULKAN);
            return static_cast<VulkanRenderer*>(rendererBackend);
        }

    private:
        bool running = true;
        float lastFrameTime = 0;
        LayerStack* layerStack;

    private:
        inline static Application* application = nullptr;
        inline static std::unique_ptr<TokiWindow> window;
        inline static RendererAPI* rendererBackend = nullptr;
        inline static RendererAPI::API rendererApi = RendererAPI::API::NONE;

        ImGuiLayer* imGuiLayer;
    };

}