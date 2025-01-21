#pragma once

#include <utility>

#include "core/base.h"
#include "input/key_codes.h"

namespace toki {

enum class KeyboardMods : u8 {
    None = 0,
    Control = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
    CapsLock = 1 << 3,
    NumLock = 1 << 4,
    Super = 1 << 5,
};

inline KeyboardMods& operator|=(KeyboardMods& lhs, const KeyboardMods rhs) {
    lhs = (KeyboardMods) (std::to_underlying(lhs) | std::to_underlying(rhs));
    return lhs;
}

inline KeyboardMods operator|(const KeyboardMods& lhs, const KeyboardMods& rhs) {
    return (KeyboardMods) (std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline KeyboardMods operator&(const KeyboardMods& lhs, const KeyboardMods& rhs) {
    return (KeyboardMods) (std::to_underlying(lhs) & std::to_underlying(rhs));
}

class Input {
public:
    Input() = delete;
    Input(void* data): m_data(data){};
    ~Input() = default;

    b8 is_key_down(KeyCode key_code) const;
    b8 is_mouse_button_pressed(MouseButton mouse_button) const;

private:
    void* m_data;
};

}  // namespace toki
