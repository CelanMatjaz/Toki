#pragma once

#include "tkpch.h"
#include "event.h"

namespace Toki {

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
        MouseButtonPressedEvent(int key, float x, float y) : Event{ EventType::MouseButtonPressed }, key{ key }, x{ x }, y{ y } {};

        int getKey() { return key; }
    private:
        int key;
        float x;
        float y;
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

    class KeyEvent : public Event {
    public:
        KeyEvent(int key, int scancode, int mods) :
            Event{ EventType::None }, key{ key }, scancode{ scancode }, mods{ mods } {}

        int getKey() { return key; }
        int getScancode() { return scancode; }
        int getMods() { return mods; }

    private:
        int key;
        int scancode;
        int mods;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(int key, int scancode, int mods) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyPressed;
        }
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int key, int scancode, int mods) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyReleased;
        }
    };

    class KeyRepeatEvent : public KeyEvent {
    public:
        KeyRepeatEvent(int key, int scancode, int mods) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyRepeat;
        }
    };
}