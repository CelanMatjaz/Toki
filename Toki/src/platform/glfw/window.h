#pragma once

#include "core/window.h"

namespace Toki {

    class TokiWindow : public Window {
    public:
        TokiWindow(const WindowConfig& windowConfig, Engine* engine);
        ~TokiWindow() override;

        virtual void pollEvents() override;
        virtual bool shouldClose() override;
        virtual void* getHandle() override;
        virtual void resize(uint32_t width, uint32_t height) override;

    private:
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void windowResizedCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow* window;
        inline static uint32_t nWindows = 0;
    };


}