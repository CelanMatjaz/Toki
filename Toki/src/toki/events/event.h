#pragma once

namespace Toki {
    class Event {
    public:
        enum class EventType {
            None,
            KeyPressed, KeyReleased,
            MouseButtonPressed, MouseMoved, MouseScrolled,
            WindowResized, WindowMinimized, WindowMaximized
        };

        Event(EventType type) : type{ type } {}
        virtual ~Event() = default;

        void setHandled() { handled = true; }
        bool isHandled() { return handled; }

    public:
        EventType getType() const { return type; }

    protected:
        EventType type;
        bool handled;
    };
}