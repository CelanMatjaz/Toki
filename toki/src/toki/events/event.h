#pragma once

#include <cstdint>
#include <functional>

namespace Toki {

enum class EventType {
    // u16[0]: Key code
    // u16[1]: Scan code
    // u16[2]: Mods
    KeyPress,

    // u16[0]: Key code
    // u16[1]: Scan code
    // u16[2]: Mods
    KeyRelease,

    // u16[0]: Key code
    // u16[1]: Scan code
    // u16[2]: Mods
    KeyRepeat,

    // u16[0]: Mouse X position
    // u16[1]: Mouse Y position
    // u16[2]: Key code
    // u16[3]: Mods
    MousePress,

    // u16[0]: Mouse X position
    // u16[1]: Mouse Y position
    // u16[2]: Key code
    // u16[3]: Mods
    MouseRelease,

    // i32[0]: Mouse X offset
    // i32[1]: Mouse Y offset
    MouseMove,

    // u16[0]: Delta scroll
    MouseScroll,

    // u16[0]: New width
    // u16[1]: New height
    WindowResize,

    // u16[0]: New width
    // u16[1]: New height
    WindowMaximize,

    // u16[0]: New width = 0
    // u16[1]: New height = 0
    WindowMinimize,

    // u16[0]: New upper left cornder X position
    // u16[1]: New upper left cornder Y position
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

using EventFunction = std::function<void(void* sender, void* receiver, const Event&)>;

class Event {
public:
    static void bindEvent(EventType eventType, void* receiver, EventFunction fn);
    static void unbindEvent(EventType eventType, void* receiver);
    static void dispatchEvent(const Event& event, void* sender);

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
