#include <sys/syscall.h>
#include <time.h>
#include <toki/core/platform/syscalls.h>
#include <unistd.h>

namespace toki {

toki::u64 get_current_time() {
	struct timespec ts;
	syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void sleep(u32 millis) {
	timespec ts{};
	ts.tv_sec  = millis / 1000;
	ts.tv_nsec = (millis % 1000) * 1000000;
	nanosleep(&ts, nullptr);
}

}  // namespace toki
