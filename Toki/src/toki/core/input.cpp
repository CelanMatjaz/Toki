#include "tkpch.h"
#include "input.h"

#include "application.h"

namespace Toki {

    bool Input::isKeyPressed(KeyboardButtons key) {
        return glfwGetKey(Application::getNativeWindow(), key);
    }

    bool Input::isMousePressed(MouseButtons button) {
        return glfwGetKey(Application::getNativeWindow(), button);
    }

    glm::vec2 Input::getCursorPosition() {
        double posX, posY;
        glfwGetCursorPos(Application::getNativeWindow(), &posX, &posY);
        return { posX, posY };
    }

}