#pragma once

#include <cstdint>
#include <string_view>

#include "toki/core/core.h"

namespace Toki {
static constexpr uint32_t MIN_WINDOW_WIDTH = 800;
static constexpr uint32_t MIN_WINDOW_HEIGHT = 600;

struct WindowConfig {
    uint16_t width = MIN_WINDOW_WIDTH;
    uint16_t height = MIN_WINDOW_HEIGHT;
    std::string title = "Window";
    bool isResizable : 1 = false;
    bool enableVSync : 1 = false;
    bool showOnCreate : 1 = false;
    bool focusOnCreate : 1 = false;
    bool floatingOnCreate : 1 = false;
    bool maximizedOnCreate : 1 = false;
    bool focusOnShow : 1 = false;
};

struct WindowDimensions {
    int width, height;
};

class Window {
public:
    static Ref<Window> create(const WindowConfig& config);

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
    virtual void* getHandle() = 0;

    static void initWindowSystem();
    static void shutdownWindowSystem();

protected:
    WindowDimensions m_dimensions;
};

}  // namespace Toki
