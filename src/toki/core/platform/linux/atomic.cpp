#include "toki/core/platform/threads/atomic.h"

#include <linux/futex.h>
#include <sys/syscall.h>
#include <toki/core/common/defines.h>
#include <unistd.h>

namespace toki {

void atomic_wait(i32* addr, i32 expected) {
	while (atomic_load(addr) == expected) {
		syscall(SYS_futex, addr, FUTEX_WAIT, expected, nullptr, nullptr, 0);
	}
}

void atomic_notify_one(i32* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE, 1, nullptr, nullptr, 0);
}

void atomic_notify_all(i32* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE, I32_MAX, nullptr, nullptr, 0);
}

}  // namespace toki
