#include <linux/futex.h>
#include <sched.h>
#include <sys/syscall.h>
#include <toki/core/common/defines.h>
#include <toki/core/platform/threads/atomic.h>
#include <toki/core/platform/threads/mutex.h>
#include <toki/core/platform/threads/semaphore.h>
#include <toki/core/platform/threads/thread.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>

#include "toki/core/common/log.h"

namespace toki {

void atomic_wait(i32* addr, i32 expected) {
	while (atomic_load(addr, ATOMIC_ACQUIRE) == expected) {
		syscall(SYS_futex, addr, FUTEX_WAIT, expected, nullptr, nullptr, 0);
	}
}

void atomic_notify_one(i32* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE, 1, nullptr, nullptr, 0);
}

void atomic_notify_all(i32* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE, I32_MAX, nullptr, nullptr, 0);
}

b8 Mutex::is_locked() {
	return atomic_load(&m_state, ATOMIC_ACQUIRE);
}

void Mutex::lock() {
	while (true) {
		i32 expected = MUTEX_UNLOCKED;

		if (atomic_compare_exchange_strong(&m_state, &expected, MUTEX_LOCKED, ATOMIC_ACQ_REL, ATOMIC_ACQUIRE)) {
			return;
		}

		while (atomic_load(&m_state, ATOMIC_ACQUIRE) == MUTEX_LOCKED) {
			syscall(SYS_futex, &m_state, FUTEX_WAIT, MUTEX_LOCKED, nullptr, nullptr, 0);
		}
	}
}

void Mutex::unlock() {
	atomic_store(&m_state, MUTEX_UNLOCKED, ATOMIC_RELEASE);
	syscall(SYS_futex, &m_state, FUTEX_WAKE, I32_MAX, nullptr, nullptr, 0);
}

b8 Mutex::try_lock() {
	i32 expected = MUTEX_UNLOCKED;
	return atomic_compare_exchange_strong(&m_state, &expected, MUTEX_LOCKED, ATOMIC_ACQ_REL, ATOMIC_ACQUIRE);
}

Semaphore::Semaphore(u32 count) {
	atomic_store(&m_count, count, ATOMIC_RELEASE);
}

void Semaphore::acquire() {
	i32 old = atomic_load(&m_count, ATOMIC_RELAXED);

	while (old <= 0 || !atomic_compare_exchange_strong(&m_count, &old, old - 1, ATOMIC_ACQ_REL, ATOMIC_ACQUIRE)) {
		atomic_wait(&m_count, old <= 0 ? 0 : old);
		old = atomic_load(&m_count, ATOMIC_RELAXED);
	}
}

void Semaphore::release() {
	atomic_fetch_add(&m_count, 1, ATOMIC_RELEASE);
	atomic_notify_all(&m_count);
}

void Thread::_start_internal(void* stack_top, void* ptr) {
	i32 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD | CLONE_CHILD_CLEARTID;

	m_data.pid = clone(Thread::_trampoline, stack_top, flags, ptr);
	if (m_data.pid == -1) {
		toki ::println(
			"{} "
			"Can't create thread {}"
			"\033[0m",
			toki ::LOG_LEVEL_STRINGS[static_cast<toki ::u32>(toki ::LogLevel ::ERROR)],
			(*__errno_location()));
	}
}

void Thread::_wait_internal() {
	atomic_wait(&m_data.ctid, 0);
}

}  // namespace toki
