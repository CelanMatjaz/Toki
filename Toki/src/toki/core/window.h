#pragma once

#include "core/core.h"
#include "string"

namespace Toki {

    struct WindowConfig {
        std::wstring title = L"Window";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool resizable = false;
    };

    class Engine;

    class Window {
    public:
        static Ref<Window> create(const WindowConfig& windowConfig, void* engine);

        Window(const WindowConfig& windowConfig, void* engine);
        virtual ~Window() = default;

        virtual void pollEvents() = 0;
        virtual bool shouldClose() = 0;
        virtual void* getHandle() = 0;
        virtual void showWindow() = 0;
        // virtual void setWindowTitle(std::wstring newTitle);
        // virtual std::wstring getWindowTitle();

        bool wasResized() { return wasResizedFlag; }
        void resetWasResized() { wasResizedFlag = false; }
        void setWasResized(bool newValue) { wasResizedFlag = newValue; }

        uint16_t getWidth() { return windowConfig.width; }
        uint16_t getHeight() { return windowConfig.height; }

        void setDimensions(int32_t width, int32_t height) {
            windowConfig.width = width;
            windowConfig.height = height;
        }

    protected:
        void* engine;
        bool resizable;
        bool wasResizedFlag = false;

        WindowConfig windowConfig;
    };

}