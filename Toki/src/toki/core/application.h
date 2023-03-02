#pragma once

#include "tkpch.h"
#include "window.h"
#include "layer_stack.h"
#include "toki/events/events.h"
#include "toki/renderer/vulkan/vulkan_renderer.h"

namespace Toki {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();
        void addLayer(Layer* layer);

        void onEvent(Event& event);

        static GLFWwindow* getNativeWindow() { return window->getHandle(); }
        static const std::unique_ptr<TokiWindow>& getWindow() { return window; }

        static  Application* getApplication() { return application; }
    private:
        bool running = true;
        float lastFrameTime = 0;
        LayerStack* layerStack;

    private:
        inline static Application* application = nullptr;
        inline static std::unique_ptr<TokiWindow> window;
        inline static VulkanRenderer* rendererBackend;
    };

}