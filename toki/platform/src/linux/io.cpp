#include <unistd.h>

#include "toki/platform/syscalls.h"

namespace toki {

i64 write(toki::NativeHandle handle, const void* data, u64 size) {
	return ::write(handle, data, size);
}

i64 read(toki::NativeHandle handle, void* data, u64 size) {
	return ::read(handle, data, size);
}

}  // namespace toki
