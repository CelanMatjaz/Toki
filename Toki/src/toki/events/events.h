#pragma once

#include "tkpch.h"
#include "event.h"

namespace Toki {

    class KeyEvent : public Event {
    private:
        uint32_t keyCode;
    };

    class MouseMoveEvent : public Event {
    public:
        MouseMoveEvent(float deltaX, float deltaY) : Event{ EventType::MouseMoved }, deltaX{ deltaX }, deltaY{ deltaY } {};

        float getDeltaX() { return deltaX; }
        float getDeltaY() { return deltaY; }
    private:
        float deltaX;
        float deltaY;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float scroll) : Event{ EventType::MouseScrolled }, scroll{ scroll } {};

        float getScroll() { return scroll; }
    private:
        float scroll;
    };

    class MouseButtonPressedEvent : public Event {
        MouseButtonPressedEvent(int key) : Event{ EventType::MouseButtonPressed }, key{ key } {};

        int ketKey() { return key; }
    private:
        int key;
    };

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(int width, int height) : Event{ EventType::WindowResized }, width{ width }, height{ height } {}

        int getWidth() { return width; }
        int getHeight() { return height; }

    private:
        int width;
        int height;
    };
}