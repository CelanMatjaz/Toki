#pragma once

#include "tkpch.h"
#include "event.h"

namespace Toki {

    class MouseEvent : public Event {
    public:
        MouseEvent(EventType eventType, int32_t x, int32_t y, uint8_t mods = 0)
            : Event(eventType), x(x), y(y), mods(mods) {}

        uint8_t getMods() { return mods; }
        int32_t getX() { return x; }
        int32_t getY() { return y; }

    private:
        int32_t x;
        int32_t y;
        uint8_t mods;
    };

    enum MouseModBits : uint8_t {
        L_BUTTON = 1 << 0,
        R_BUTTON = 1 << 1,
        SHIFT = 1 << 2,
        CONTROL = 1 << 3,
        M_BUTTON = 1 << 4,
        X_BUTTON1 = 1 << 5,
        X_BUTTON2 = 1 << 6,
    };

    class MouseScrollEvent : public MouseEvent {
    public:
        MouseScrollEvent(int16_t delta, int32_t x, int32_t y, uint8_t mods = 0)
            : MouseEvent(EventType::MouseScroll, x, y, mods), delta(delta) {};
        int16_t getScrollDelta() { return delta; }

    private:
        int16_t delta;
    };

    enum class MouseButton {
        Left, Right, Middle
    };

    class MouseButtonEvent {
    public:
        MouseButtonEvent(MouseButton button) : button(button) {}
        MouseButton getButton() { return button; }
    private:
        MouseButton button;
    };

    class MouseButtonPressEvent : public MouseEvent, MouseButtonEvent {
    public:
        MouseButtonPressEvent(MouseButton button, int32_t x, int32_t y, uint8_t mods = 0)
            : MouseEvent(EventType::MouseButtonPress, x, y, mods), MouseButtonEvent(button) {};
    };

    class MouseButtonReleaseEvent : public MouseEvent, MouseButtonEvent {
    public:
        MouseButtonReleaseEvent(MouseButton button, int32_t x, int32_t y, uint8_t mods = 0)
            : MouseEvent(EventType::MouseButtonRelease, x, y, mods), MouseButtonEvent(button) {};
    };

    class MouseButtonDoubleClickEvent : public MouseEvent, MouseButtonEvent {
    public:
        MouseButtonDoubleClickEvent(MouseButton button, int32_t x, int32_t y, uint8_t mods = 0)
            : MouseEvent(EventType::MouseButtonDoubleClick, x, y, mods), MouseButtonEvent(button) {};
    };

    class MouseMoveEvent : public MouseEvent {
    public:
        MouseMoveEvent(int32_t x, int32_t y, uint8_t mods = 0)
            : MouseEvent(EventType::MouseMove, x, y, mods) {}
    };

    class MouseEnterEvent : public Event {
    public:
        MouseEnterEvent() : Event(EventType::MouseEnter) {};
    };

    class MouseLeaveEvent : public Event {
    public:
        MouseLeaveEvent() : Event(EventType::MouseLeave) {};
    };













    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(int width, int height) : Event{ EventType::WindowResize }, width{ width }, height{ height } {}

        int getWidth() { return width; }
        int getHeight() { return height; }

    private:
        int width;
        int height;
    };

    class WindowFocusEvent : public Event {
    public:
        WindowFocusEvent() : Event(EventType::WindowFocus) { }
    };

    class WindowMinimizeEvent : public Event {
    public:
        WindowMinimizeEvent() : Event(EventType::WindowMinimize) { }
    };

    class WindowMaximizeEvent : public Event {
    public:
        WindowMaximizeEvent() : Event(EventType::WindowMaximize) { }
    };

    enum KeyModBits : uint8_t {
        LEFT_SHIFT = 1 << 0,
        LEFT_CTRL = 1 << 1,
        LEFT_MENU = 1 << 2,
        RIGHT_SHIFT = 1 << 3,
        RIGHT_CTRL = 1 << 4,
        RIGHT_MENU = 1 << 5,
    };

    class KeyEvent : public Event {
    public:
        KeyEvent(int key, int scancode, uint8_t mods) :
            Event{ EventType::None }, key{ key }, scancode{ scancode }, mods{ mods } {}

        int getKey() { return key; }
        int getScancode() { return scancode; }
        uint8_t getMods() { return mods; }

    private:
        int key;
        int scancode;
        uint8_t mods;
    };

    class KeyPressEvent : public KeyEvent {
    public:
        KeyPressEvent(int key, int scancode, int mods = 0) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyPress;
        }
    };

    class KeyReleaseEvent : public KeyEvent {
    public:
        KeyReleaseEvent(int key, int scancode, int mods = 0) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyRelease;
        }
    };

    class KeyRepeatEvent : public KeyEvent {
    public:
        KeyRepeatEvent(int key, int scancode, int mods = 0, uint32_t repeats = 0) : KeyEvent(key, scancode, mods) {
            type = EventType::KeyRepeat;
        }
    };
}