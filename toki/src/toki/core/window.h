#pragma once

#include <cstdint>
#include <string_view>

#include "core.h"

namespace Toki {
static constexpr uint32_t MIN_WINDOW_WIDTH = 800;
static constexpr uint32_t MIN_WINDOW_HEIGHT = 600;

struct WindowConfig {
    uint16_t width = MIN_WINDOW_WIDTH;
    uint16_t height = MIN_WINDOW_HEIGHT;
    std::string title = "Window";
    bool isResizable : 1 = false;
    bool enableVSync : 1 = false;
};

struct WindowDimensions {
    uint16_t width, height;
};

class Window {
public:
    virtual ~Window() = default;

    virtual void pollEvents() = 0;
    virtual bool shouldClose() = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void close() = 0;
    virtual void resize(uint16_t width, uint16_t height) = 0;
    virtual void setTitle(std::string_view title) = 0;
    virtual void setFloating(bool floating) = 0;

    const WindowDimensions& getDimensions();

protected:
    WindowDimensions m_dimensions;
    Ref<void> m_handle;
};

}  // namespace Toki
