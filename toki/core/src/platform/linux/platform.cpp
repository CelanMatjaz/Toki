#include "toki/core/platform/platform.h"

#include <toki/core/common/assert.h>
#include <unistd.h>

namespace toki {

void write(NativeHandle handle, const void* buffer, u32 size) {
	TK_ASSERT(::write(handle, buffer, size), "Could not write to fd");
}

}  // namespace toki
