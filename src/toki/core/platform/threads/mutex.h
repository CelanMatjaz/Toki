#pragma once

#include <toki/core/common/macros.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/platform/threads/atomic.h>
#include <toki/core/types.h>

namespace toki {

class Mutex {
public:
	Mutex() {
		atomic_store(&m_state, MUTEX_UNLOCKED);
	}

	DELETE_COPY(Mutex)
	DEFAULT_MOVE(Mutex);

	b8 is_locked();
	void lock();
	b8 try_lock();
	void unlock();

	i32& state() {
		return m_state;
	}

	static constexpr i32 MUTEX_UNLOCKED = 0;
	static constexpr i32 MUTEX_LOCKED	= 1;

private:
	i32 m_state;
};

class ScopedLock {
public:
	ScopedLock(Mutex& mutex): m_mutex(mutex) {
		m_mutex.lock();
	}

	~ScopedLock() {
		m_mutex.unlock();
	}

	DELETE_COPY(ScopedLock);
	DELETE_MOVE(ScopedLock);

private:
	Mutex& m_mutex;
};

}  // namespace toki
