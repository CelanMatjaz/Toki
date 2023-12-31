#include "tkpch.h"
#include "window.h"
#include "core/engine.h"
#include "events/events.h"

namespace Toki {

    TokiWindow::TokiWindow(const WindowConfig& windowConfig, Engine* engine) : Window(windowConfig, engine) {
        if (nWindows == 0) glfwInit();
        ++nWindows;

        glfwWindowHint(GLFW_RESIZABLE, windowConfig.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, windowResizedCallback);
        glfwSetKeyCallback(window, keyCallback);
    }

    TokiWindow::~TokiWindow() {

    }

    void TokiWindow::pollEvents() {
        glfwPollEvents();
    }

    bool TokiWindow::shouldClose() {
        return glfwWindowShouldClose(window) ? true : false;
    }

    void* TokiWindow::getHandle() {
        return window;
    }

    void TokiWindow::resize(uint32_t width, uint32_t height) {
        // glfwSetWindowSize(window, width, height);
        // WindowResizeEvent ev(width, height);
        // engine->onEvent(ev);
    }

    void TokiWindow::windowResizedCallback(GLFWwindow* window, int width, int height) {
        auto win = (TokiWindow*) (glfwGetWindowUserPointer(window));
        win->setWasResized(true);
        win->width = width;
        win->height = height;
        WindowResizeEvent ev(width, height);
        win->engine->onEvent(ev);
    }

    void TokiWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    }


}