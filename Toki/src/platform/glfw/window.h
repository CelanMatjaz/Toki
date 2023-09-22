#pragma once

#include "core/window.h"

#ifdef TK_GLFW

namespace Toki {

    class TokiWindow : public Window {
    public:
        TokiWindow(const WindowConfig& windowConfig, void* engine);
        ~TokiWindow() override;

        virtual void pollEvents() override;
        virtual bool shouldClose() override;
        virtual void* getHandle() override;
        virtual void showWindow() override;

    private:
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void windowResizedCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow* window;
        inline static uint32_t nWindows = 0;
    };


}

#endif