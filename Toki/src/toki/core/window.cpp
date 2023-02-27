#include "window.h"

#include "tkpch.h"

namespace Toki {

    std::unique_ptr<TokiWindow> TokiWindow::createWindow(const char* title, uint32_t width, uint32_t height, bool resizable) {
        std::unique_ptr<TokiWindow> window = std::make_unique<TokiWindow>();

        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window->handle = glfwCreateWindow(width, height, title, nullptr, nullptr);

        glfwMakeContextCurrent(window->handle);
        glfwSetWindowUserPointer(window->handle, window.get());

        return window;
    }

    TokiWindow::WindowDimensions TokiWindow::getWindowDimensions() {
        int width, height;
        glfwGetWindowSize(handle, &width, &height);
        return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

}