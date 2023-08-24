#pragma once

#include "core/core.h"
#include "core/engine.h"
#include "string"

namespace Toki {

    struct WindowConfig {
        std::string title = "Window";
        uint16_t width = 1280, height = 720;
        bool resizable = false;
    };

    class Engine;

    class Window {
    public:
        static Ref<Window> create(const WindowConfig& windowConfig, Engine* engine);

        Window(const WindowConfig& windowConfig, Engine* engine);
        virtual ~Window() = default;

        virtual void pollEvents() = 0;
        virtual bool shouldClose() = 0;
        virtual void* getHandle() = 0;

        bool wasResized() { return wasResizedFlag; }
        void resetWasResized() { wasResizedFlag = false; }
        void setWasResized(bool newValue) { wasResizedFlag = newValue; }

        uint16_t getWidth() { return width; }
        uint16_t getHeight() { return height; }

    protected:
        Engine* engine;
        uint16_t width, height;
        bool resizable;
        bool wasResizedFlag = false;
    };

}