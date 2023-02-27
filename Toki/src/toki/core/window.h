#pragma once

#include "tkpch.h"

namespace Toki {

    class TokiWindow {
    public:
        TokiWindow() = default;
        ~TokiWindow() = default;

        static std::unique_ptr<TokiWindow> createWindow(const char* title, uint32_t width, uint32_t height, bool resizable = false);

        GLFWwindow* getHandle() { return handle; }

    private:
        struct WindowDimensions {
            uint32_t width, height;
        };

    public:
        WindowDimensions getWindowDimensions() const { return dimensions; }

        bool wasResized() { return this->_wasResized; }
        void resetResizedFlag() { _wasResized = false; }

        static void windowResizedCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow* handle;
        WindowDimensions dimensions;
        bool _wasResized = false;
    };

}