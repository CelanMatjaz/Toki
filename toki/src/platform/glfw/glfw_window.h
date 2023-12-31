#pragma once

#include "GLFW/glfw3.h"
#include "tkpch.h"
#include "toki/core/window.h"

#ifdef WINDOW_SYSTEM_GLFW

namespace Toki {

class GlfwWindow : public Window {
public:
    GlfwWindow() = delete;
    GlfwWindow(WindowConfig windowConfig);
    virtual ~GlfwWindow();

    virtual bool shouldClose() const override;
    virtual void pollEvents() const override;
    virtual WindowDimensions getDimensions() const override;

    virtual void* getHandle() const override { return windowHandle; }

private:
    GLFWwindow* windowHandle;

    inline static uint32_t nWindows = 0;
};

}  // namespace Toki

#endif