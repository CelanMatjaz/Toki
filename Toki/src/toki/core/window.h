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
        WindowDimensions getWindowDimensions();

    private:
        GLFWwindow* handle;
    };

}