#if defined(TK_PLATFORM_LINUX)

#include "platform.h"

#include <csignal>

namespace toki {

void debug_break() {
    raise(SIGTRAP);
}

}  // namespace toki

#endif
