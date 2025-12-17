#include "toki/core/platform/threads/mutex.h"

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace toki {

void Mutex::lock() {
	while (true) {
		MutexState expected = MUTEX_UNLOCKED;

		if (atomic_compare_exchange_strong(&m_state, &expected, MUTEX_LOCKED)) {
			return;
		}

		while (atomic_load(&m_state) == MUTEX_LOCKED) {
			syscall(SYS_futex, &m_state, FUTEX_WAIT, MUTEX_LOCKED, nullptr, nullptr, 0);
		}
	}
}

void Mutex::unlock() {
	atomic_store(&m_state, MUTEX_UNLOCKED);
	syscall(SYS_futex, &m_state, FUTEX_WAKE, 1, nullptr, nullptr, 0);
}

b8 Mutex::try_lock() {
	MutexState expected = MUTEX_UNLOCKED;
	i32 value			= atomic_load(&m_state);
	return atomic_compare_exchange_strong(&value, &expected, MUTEX_LOCKED);
}

}  // namespace toki
