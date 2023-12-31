#pragma once

#include <cstdint>
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
};

class Event {
public:
    Event() = delete;
    Event(EventType eventType, uint64_t data = 0);
    ~Event() = default;

    EventType getType() const;
    int16_t getData(uint8_t word = 0) const;
    bool getIsHandled() const;
    void handle();

private:
    EventType eventType;
    uint64_t data;
    bool isHandled = false;
};

}  // namespace Toki
