#pragma once 

#include "tkpch.h"
#include "key_codes.h"

namespace Toki {

    class Input {
    public:
        static bool isKeyPressed(KeyboardButtons key);
        static bool isMousePressed(MouseButtons button);
        static glm::vec2 getCursorPosition();
    };

}