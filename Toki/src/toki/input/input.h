#pragma once

#include "scancodes.h"
#include "glm/glm.hpp"

namespace Toki {

    class Input {
    public:
        static bool isKeyPressed(ScanCode scanCode);
        static bool isKeyPressed(KeyCode keyCode);
        static bool isMouseButtonPressed(MouseButton mouseButton);

        static glm::ivec2 getMousePosition();
    };

}