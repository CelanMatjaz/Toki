#include <csignal>

#include "../platform.h"

namespace toki {

void debug_break() {
    raise(SIGTRAP);
}

}  // namespace toki
