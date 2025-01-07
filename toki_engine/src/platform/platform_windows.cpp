#include "platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#define NOMINMAX
#include <windows.h>

namespace toki {

void debug_break() {
    DebugBreak();
}

}  // namespace toki

#endif
