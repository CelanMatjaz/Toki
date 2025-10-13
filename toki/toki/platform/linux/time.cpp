#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "toki/platform/syscalls.h"

namespace toki::platform {

toki::u64 get_current_time() {
	struct timespec ts;
	syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

}  // namespace toki::platform
