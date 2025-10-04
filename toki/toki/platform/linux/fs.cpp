#include <linux/limits.h>
#include <toki/platform/errors.h>
#include <unistd.h>

#include "toki/platform/syscalls.h"

namespace toki::platform {

// toki::Expected<toki::Path, PlatformError> getcwd() {
// 	char buf[PATH_MAX]{};
// 	i64 result = ::syscall(SYS_getcwd, buf, sizeof(buf));
// 	if (result == -1) {
// 		return PlatformError::Unknown;
// 	}
//
// 	return toki::Path(buf);
// }

}  // namespace toki::platform
