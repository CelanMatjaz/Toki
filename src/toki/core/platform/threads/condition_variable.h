#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/platform/threads/mutex.h>

namespace toki {

class ConditionVariable {
public:
	void wait(ScopedLock& lock) {
		TK_ASSERT(lock.is_locked(), "Mutex should be locked");
		atomic_wait(&lock.state(), Mutex::MUTEX_LOCKED);
	}

	template <typename Predicate>
	void wait(Mutex& lock, Predicate pred) {
		while (!pred()) {
			wait(lock);
		}
	}

	void notify_one() {
		atomic_notify_one(&)
	}

	void notify_all();

private:
	b32i m_cond{};
};

}  // namespace toki
