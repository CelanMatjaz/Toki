#include "tkpch.h"
#include "window.h"
#include "core/engine.h"

#ifdef TK_GLFW

namespace Toki {

    TokiWindow::TokiWindow(const WindowConfig& windowConfig, void* engine) : Window(windowConfig, engine) {
        if (nWindows == 0) glfwInit();
        ++nWindows;

        glfwWindowHint(GLFW_RESIZABLE, windowConfig.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        std::string title(windowConfig.title.size(), ' ');
        for (uint32_t i = 0; i < windowConfig.title.size(); ++i) {
            title[i] = (char) windowConfig.title[i];
        }

        window = glfwCreateWindow(windowConfig.width, windowConfig.height, title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, windowResizedCallback);
        glfwSetKeyCallback(window, keyCallback);
    }

    TokiWindow::~TokiWindow() {

    }

    void TokiWindow::showWindow() {

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

#endif