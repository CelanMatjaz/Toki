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

    public:
        EventType getType() const { return type; }

    protected:
        EventType type;
    };
}