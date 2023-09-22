#include "tkpch.h"
#include "scancodes.h"

namespace Toki {

    ScanCode mapKeyCode(KeyCode code, std::unordered_map<KeyCode, ScanCode> layoutMap) {
        if (layoutMap.contains(code)) return layoutMap[code];
        return ScanCode::SCAN_UNKNOWN;
    }

    // TODO: verify mappings
    int mapToWindowsKeyCode(ScanCode scanCode) {
        switch (scanCode) {
            case ScanCode::SCAN_0: {
                return VK_NUMPAD0;
            }
            case ScanCode::SCAN_1: {
                return VK_NUMPAD1;
            }
            case ScanCode::SCAN_2: {
                return VK_NUMPAD0;
            }
            case ScanCode::SCAN_3: {
                return VK_NUMPAD3;
            }
            case ScanCode::SCAN_4: {
                return VK_NUMPAD4;
            }
            case ScanCode::SCAN_5: {
                return VK_NUMPAD5;
            }
            case ScanCode::SCAN_6: {
                return VK_NUMPAD6;
            }
            case ScanCode::SCAN_7: {
                return VK_NUMPAD7;
            }
            case ScanCode::SCAN_8: {
                return VK_NUMPAD8;
            }
            case ScanCode::SCAN_9: {
                return VK_NUMPAD9;
            }
            case ScanCode::SCAN_A: {
                return 65;
            }
            case ScanCode::SCAN_B: {
                return 66;
            }
            case ScanCode::SCAN_C: {
                return 67;
            }
            case ScanCode::SCAN_D: {
                return 68;
            }
            case ScanCode::SCAN_E: {
                return 69;
            }
            case ScanCode::SCAN_F: {
                return 70;
            }
            case ScanCode::SCAN_G: {
                return 71;
            }
            case ScanCode::SCAN_H: {
                return 72;
            }
            case ScanCode::SCAN_I: {
                return 73;
            }
            case ScanCode::SCAN_J: {
                return 74;
            }
            case ScanCode::SCAN_K: {
                return 75;
            }
            case ScanCode::SCAN_L: {
                return 76;
            }
            case ScanCode::SCAN_M: {
                return 77;
            }
            case ScanCode::SCAN_N: {
                return 78;
            }
            case ScanCode::SCAN_O: {
                return 79;
            }
            case ScanCode::SCAN_P: {
                return 80;
            }
            case ScanCode::SCAN_Q: {
                return 81;
            }
            case ScanCode::SCAN_R: {
                return 82;
            }
            case ScanCode::SCAN_S: {
                return 83;
            }
            case ScanCode::SCAN_T: {
                return 84;
            }
            case ScanCode::SCAN_U: {
                return 85;
            }
            case ScanCode::SCAN_V: {
                return 86;
            }
            case ScanCode::SCAN_W: {
                return 87;
            }
            case ScanCode::SCAN_X: {
                return 88;
            }
            case ScanCode::SCAN_Y: {
                return 89;
            }
            case ScanCode::SCAN_Z: {
                return 90;
            }
            case ScanCode::SCAN_SPACE: {
                return VK_SPACE;
            }
            case ScanCode::SCAN_APOSTROPHE: {
                return 41;
            }
            case ScanCode::SCAN_COMMA: {
                return 52;
            }
            case ScanCode::SCAN_MINUS: {
                return 12;
            }
            case ScanCode::SCAN_EQUAL: {
                return 13;
            }
            case ScanCode::SCAN_LEFT_BRACKET: {
                return 26;
            }
            case ScanCode::SCAN_BACKSLASH: {
                return 14;
            }
            case ScanCode::SCAN_RIGHT_BRACKET: {
                return 27;
            }
            case ScanCode::SCAN_GRAVE_ACCENT: {
                return 39;
            }
            case ScanCode::SCAN_F1: {
                return VK_F1;
            }
            case ScanCode::SCAN_F2: {
                return VK_F2;
            }
            case ScanCode::SCAN_F3: {
                return VK_F3;
            }
            case ScanCode::SCAN_F4: {
                return VK_F4;
            }
            case ScanCode::SCAN_F5: {
                return VK_F5;
            }
            case ScanCode::SCAN_F6: {
                return VK_F6;
            }
            case ScanCode::SCAN_F7: {
                return VK_F7;
            }
            case ScanCode::SCAN_F8: {
                return VK_F8;
            }
            case ScanCode::SCAN_F9: {
                return VK_F9;
            }
            case ScanCode::SCAN_F10: {
                return VK_F10;
            }
            case ScanCode::SCAN_F11: {
                return VK_F11;
            }
            case ScanCode::SCAN_F12: {
                return VK_F12;
            }
            case ScanCode::SCAN_F13: {
                return VK_F13;
            }
            case ScanCode::SCAN_F14: {
                return VK_F14;
            }
            case ScanCode::SCAN_F15: {
                return VK_F15;
            }
            case ScanCode::SCAN_F16: {
                return VK_F16;
            }
            case ScanCode::SCAN_F17: {
                return VK_F17;
            }
            case ScanCode::SCAN_F18: {
                return VK_F18;
            }
            case ScanCode::SCAN_F19: {
                return VK_F19;
            }
            case ScanCode::SCAN_F20: {
                return VK_F20;
            }
            case ScanCode::SCAN_F21: {
                return VK_F21;
            }
            case ScanCode::SCAN_F22: {
                return VK_F22;
            }
            case ScanCode::SCAN_F23: {
                return VK_F23;
            }
            case ScanCode::SCAN_F24: {
                return VK_F24;
            }
            case ScanCode::SCAN_ESCAPE: {
                return VK_ESCAPE;
            }
            case ScanCode::SCAN_ENTER: {
                return VK_RETURN;
            }
            case ScanCode::SCAN_TAB: {
                return VK_TAB;
            }
            case ScanCode::SCAN_BACKSPACE: {
                return VK_BACK;
            }
            case ScanCode::SCAN_INSERT: {
                return VK_INSERT;
            }
            case ScanCode::SCAN_DELETE: {
                return VK_DELETE;
            }
            case ScanCode::SCAN_PAGE_UP: {
                return VK_PRIOR;
            }
            case ScanCode::SCAN_PAGE_DOWN: {
                return VK_NEXT;
            }
            case ScanCode::SCAN_HOME: {
                return VK_HOME;
            }
            case ScanCode::SCAN_END: {
                return VK_END;
            }
            case ScanCode::SCAN_CAPS_LOCK: {
                return VK_CAPITAL;
            }
            case ScanCode::SCAN_SCROLL_LOCK: {
                return VK_SCROLL;
            }
            case ScanCode::SCAN_NUM_LOCK: {
                return VK_NUMLOCK;
            }
            case ScanCode::SCAN_PRINT_SCREEN: {
                return VK_SNAPSHOT;
            }
            case ScanCode::SCAN_PAUSE: {
                return VK_PAUSE;
            }
            case ScanCode::SCAN_RIGHT: {
                return VK_RIGHT;
            }
            case ScanCode::SCAN_LEFT: {
                return VK_LEFT;
            }
            case ScanCode::SCAN_UP: {
                return VK_UP;
            }
            case ScanCode::SCAN_DOWN: {
                return VK_DOWN;
            }
            case ScanCode::SCAN_KEYPAD_0: {
                return VK_NUMPAD0;
            }
            case ScanCode::SCAN_KEYPAD_1: {
                return VK_NUMPAD1;
            }
            case ScanCode::SCAN_KEYPAD_2: {
                return VK_NUMPAD2;
            }
            case ScanCode::SCAN_KEYPAD_3: {
                return VK_NUMPAD3;
            }
            case ScanCode::SCAN_KEYPAD_4: {
                return VK_NUMPAD4;
            }
            case ScanCode::SCAN_KEYPAD_5: {
                return VK_NUMPAD5;
            }
            case ScanCode::SCAN_KEYPAD_6: {
                return VK_NUMPAD6;
            }
            case ScanCode::SCAN_KEYPAD_7: {
                return VK_NUMPAD7;
            }
            case ScanCode::SCAN_KEYPAD_8: {
                return VK_NUMPAD8;
            }
            case ScanCode::SCAN_KEYPAD_9: {
                return VK_NUMPAD9;
            }
            case ScanCode::SCAN_KEYPAD_SLASH: {
                return VK_DIVIDE;
            }
            case ScanCode::SCAN_KEYPAD_ASTERISK: {
                return VK_MULTIPLY;
            }
            case ScanCode::SCAN_KEYPAD_MINUS: {
                return VK_SUBTRACT;
            }
            case ScanCode::SCAN_KEYPAD_PLUS: {
                return VK_ADD;
            }
            case ScanCode::SCAN_KEYPAD_ENTER: {
                return VK_RETURN;
            }
            case ScanCode::SCAN_KEYPAD_COMMA: {
                return VK_DECIMAL;
            }
            case ScanCode::SCAN_LEFT_SHIFT: {
                return VK_LSHIFT;
            }
            case ScanCode::SCAN_LEFT_CONTROL: {
                return VK_LCONTROL;
            }
            case ScanCode::SCAN_LEFT_ALT: {
                return VK_LMENU;
            }
            case ScanCode::SCAN_LEFT_SUPER: {
                return VK_LWIN;
            }
            case ScanCode::SCAN_RIGHT_SHIFT: {
                return VK_RSHIFT;
            }
            case ScanCode::SCAN_RIGHT_CONTROL: {
                return VK_RCONTROL;
            }
            case ScanCode::SCAN_RIGHT_ALT: {
                return VK_RMENU;
            }
            case ScanCode::SCAN_RIGHT_SUPER: {
                return VK_RWIN;
            }
            case ScanCode::SCAN_MENU: {
                return 0;
            }
        }

        return 0;
    }

}