#include "platform/platform.h"

#if defined(TK_PLATFORM_LINUX)


#include "../core/assert.h"
#include "../core/base.h"

namespace toki {

namespace platform {

void debug_break() {
    raise(SIGTRAP);
}


}  // namespace platform

}  // namespace toki

#endif
