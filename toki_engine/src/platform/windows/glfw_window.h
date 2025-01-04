// Use file on linux while using glfw
#if defined(TK_PLATFORM_WINDOWS) || defined(TK_PLATFORM_LINUX)

#pragma once

#include <GLFW/glfw3.h>

#include "engine/window.h"

namespace toki {

class GlfwWindow : public Window {
public:
    GlfwWindow() = delete;
    GlfwWindow(const Config& config);
    virtual ~GlfwWindow();

    virtual bool should_close() const override;
    virtual void* get_handle() const override;

private:
    GLFWwindow* m_handle{};
};

}  // namespace toki

#endif
