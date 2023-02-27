#pragma once

#include "tkpch.h"
#include "window.h"
#include "toki/renderer/vulkan/vulkan_renderer.h"

namespace Toki {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();

        static GLFWwindow* getNativeWindow() { return window->getHandle(); }
        static const std::unique_ptr<TokiWindow>& getWindow() { return window; }

        virtual void onUpdate(float deltaTime) = 0;

    private:
        bool running = true;
        float lastFrameTime = 0;

    private:
        inline static Application* application = nullptr;
        inline static std::unique_ptr<TokiWindow> window;
        inline static VulkanRenderer* rendererBackend;
    };

}