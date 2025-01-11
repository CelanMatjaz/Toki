#pragma once

#include "core/base.h"
#include "input/key_codes.h"

namespace toki {

class Input {
public:
    Input() = delete;
    Input(void* data): m_data(data) {};
    ~Input() = default;

    b8 is_key_down(KeyCode key_code);
    b8 is_mouse_button_pressed(MouseButton mouse_button);

private:
    void* m_data;
};

}  // namespace toki
