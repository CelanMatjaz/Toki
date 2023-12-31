#pragma once

#include "cstdint"

namespace Toki {

struct WindowConfig {
    const wchar_t* title;
    uint16_t width, height;
    bool resizable;
    bool enableVSync;
};

class Window {
public:
    Window() = default;
    virtual ~Window() = default;

    virtual bool shouldClose() const = 0;
    virtual void pollEvents() = 0;
    virtual void close() = 0;

    struct WindowDimensions {
        int32_t width, height;
    };

    virtual WindowDimensions getDimensions() const = 0;
    virtual void* getHandle() const = 0;
};

}  // namespace Toki
