// Use file on linux while using glfw
#if defined(TK_PLATFORM_WINDOWS) || defined(TK_PLATFORM_LINUX)

#pragma once

#include <GLFW/glfw3.h>

#include "engine/window.h"

namespace toki {

class glfw_window : public window {
public:
    glfw_window() = delete;
    glfw_window(const config& config);
    virtual ~glfw_window();

    virtual Vec2i get_dimensions() const override;

    virtual bool should_close() const override;
    virtual void* get_handle() const override;

private:
    GLFWwindow* m_handle{};
};

}  // namespace toki

#endif
