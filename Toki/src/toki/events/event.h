#pragma once

namespace Toki {

    enum class EventType {
        None,
        KeyPress, KeyRelease, KeyRepeat,
        MouseButtonPress, MouseButtonRelease, MouseButtonDoubleClick, MouseMove, MouseScroll, MouseEnter, MouseLeave,
        WindowResize, WindowFocus, WindowMinimize, WindowMaximize
    };

    class Event {
    public:
        Event(EventType type) : type{ type } {}
        virtual ~Event() = default;

        void setHandled() { handled = true; }
        bool isHandled() { return handled; }

    public:
        EventType getType() const { return type; }

    protected:
        EventType type;
        bool handled = false;
    };
}