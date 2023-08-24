#include "tkpch.h"
#include "window.h"
#include "core/engine.h"

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

    void TokiWindow::windowResizedCallback(GLFWwindow* window, int width, int height) {
        auto win = (TokiWindow*) (glfwGetWindowUserPointer(window));
        win->setWasResized(true);
    }

    void TokiWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    }


}