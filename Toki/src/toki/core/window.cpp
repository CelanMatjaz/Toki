#include "window.h"

#include "tkpch.h"
#include "application.h"
#include "toki/events/events.h"

namespace Toki {

    std::unique_ptr<TokiWindow> TokiWindow::createWindow(const char* title, uint32_t width, uint32_t height, bool resizable) {
        std::unique_ptr<TokiWindow> window = std::make_unique<TokiWindow>();

        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window->handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
        window->dimensions = { width, height };

        glfwMakeContextCurrent(window->handle);
        glfwSetWindowUserPointer(window->handle, Application::getApplication());
        glfwSetFramebufferSizeCallback(window->handle, windowResizedCallback);
        glfwSetKeyCallback(window->handle, keyCallback);

        return window;
    }

    void TokiWindow::windowResizedCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->getWindow()->dimensions = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        app->getWindow()->_wasResized = true;
        WindowResizeEvent event(width, height);
        app->onEvent(event);
    }

    void TokiWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key, scancode, mods);
                app->onEvent(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key, scancode, mods);
                app->onEvent(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyRepeatEvent event(key, scancode, mods);
                app->onEvent(event);
                break;
            }
        }
    }

}