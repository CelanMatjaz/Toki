#pragma once

#include <utility>

#include "events/event.h"
#include "input/input.h"
#include "input/key_codes.h"

namespace toki {

class KeyEvent : public Event {
public:
    KeyEvent(EventType type): Event(type) {}

    KeyCode get_key() const {
        return (KeyCode) m_data.u16[0];
    }

    u16 get_scan_code() const {
        return m_data.u16[1];
    }

    bool operator==(const KeyCode& key_code) {
        return m_data.u16[0] == std::to_underlying(key_code);
    }

    KeyboardMods get_mods() const {
        return (KeyboardMods) m_data.u16[1];
    };
};

class KeyPressEvent : public KeyEvent {
    KeyPressEvent(): KeyEvent(EventType::KeyPress) {}
};

class KeyReleaseEvent : public KeyEvent {
    KeyReleaseEvent(): KeyEvent(EventType::KeyRelease) {}
};

class KeyRepeatEvent : public KeyEvent {
    KeyRepeatEvent(): KeyEvent(EventType::KeyRepeat) {}
};

}  // namespace toki
