#include "tkpch.h"
#include "input.h"
#include "core/assert.h"
#include "core/engine.h"
#include "scancodes.h"

#ifdef TK_WIN32
#include "Windows.h"
#else 

#endif

namespace Toki {

    bool Input::isKeyPressed(ScanCode scanCode) {
        return GetAsyncKeyState(mapToWindowsKeyCode(scanCode));
    }

    bool Input::isKeyPressed(KeyCode keyCode) {
        // TODO: implement



        return false;
    }

    bool Input::isMouseButtonPressed(MouseButton mouseButton) {
        // TODO: implement


        return false;
    }

    glm::ivec2 Input::getMousePosition() {
#ifdef TK_WIN32
        POINT point;
        GetCursorPos(&point);
        ScreenToClient((HWND) Engine::getWindow()->getHandle(), &point);
        return { point.x, point.y };
#else 
        double x, y;
        glfwGetCursorPos((GLFWwindow*) Engine::getWindow()->getHandle(), &x, &y);
        return { x, y };
#endif
    }


}