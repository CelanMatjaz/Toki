#pragma once

#include "platform.h"

#ifdef TK_WINDOW_SYSTEM_GLFW

#include <GLFW/glfw3.h>

#include "toki/core/window.h"

namespace Toki {

class GlfwWindow : public Window {
public:
    GlfwWindow(const WindowConfig& windowConfig);
    virtual ~GlfwWindow() override;

    virtual void pollEvents() override;
    virtual bool shouldClose() override;

    virtual void show() override;
    virtual void hide() override;
    virtual void close() override;
    virtual void resize(uint16_t width, uint16_t height) override;
    virtual void setTitle(std::string_view title) override;
    virtual void setFloating(bool floating) override;

    virtual void* getHandle() override;

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowResizedCallback(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* m_windowHandle = nullptr;
};

}  // namespace Toki

#endif