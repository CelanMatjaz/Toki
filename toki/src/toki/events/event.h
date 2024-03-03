#pragma once

#include <cstdint>
#include <functional>

namespace Toki {

enum class EventType {
    KeyPress,        // W0: Key code
    KeyRelease,      // W0: Key code
    KeyRepeat,       // W0: Key code
    KeyPressChar,    // W0: Character
    KeyReleaseChar,  // W0: Character
    KeyRepeatChar,   // W0: Character
    // W0: Key code
    // W1: Mouse X position
    // W2: Mouse Y position
    MousePress,
    // W0: Key code
    // W1: Mouse X position
    // W2: Mouse Y position
    MouseRelease,
    // W0: Key code
    // W1: Mouse X position
    // W2: Mouse Y position
    MouseMove,
    MouseScroll,  // W0: Delta scroll
    // W0: New width
    // W1: New height
    WindowResize,
    // W0: New width
    // W1: New height
    WindowMaximize,
    // W0: New width = 0
    // W1: New height = 0
    WindowMinimize,
    // W0: New upper left cornder X position
    // W1: New upper left cornder Y position
    WindowMove,
    WindowFocus,
    WindowBlur,
    WindowClose,
    EVENT_COUNT
};

union EventData {
    uint64_t u64;
    int64_t i64;

    uint32_t u32[2];
    int32_t i32[2];

    uint16_t u16[4];
    int16_t i16[4];

    uint8_t u8[8];
    int8_t i8[8];
};

class Event;

using EventFunction = std::function<void(void* sender, void* receiver, Event&)>;

class Event {
public:
    static void bindEvent(EventType eventType, void* receiver, EventFunction fn);
    static void unbindEvent(EventType eventType, void* receiver);
    static void dispatchEvent(Event& event, void* sender);

    Event() = delete;
    Event(EventType eventType, EventData data = {});
    ~Event() = default;

    EventType getType() const;
    EventData getData() const;
    bool isHandled() const;
    void setHandled();

private:
    EventType m_eventType;
    EventData m_data;
    bool m_isHandled = false;
};

}  // namespace Toki
