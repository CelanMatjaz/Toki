#include "tkpch.h"
#include "scancodes.h"

namespace Toki {

    ScanCode mapKeyCode(KeyCode code, std::unordered_map<KeyCode, ScanCode> layoutMap) {
        if (layoutMap.contains(code)) return layoutMap[code];
        return ScanCode::SCAN_UNKNOWN;
    }

}