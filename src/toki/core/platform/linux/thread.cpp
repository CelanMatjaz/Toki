#include <linux/futex.h>
#include <linux/sched.h>
#include <sched.h>
#include <sys/wait.h>
#include <toki/core/common/print.h>
#include <toki/core/platform/threads/thread.h>

#include <cerrno>

namespace toki {

void Thread::_start_internal(void* stack_top, void* ptr) {
	i32 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD | CLONE_CHILD_CLEARTID;

	m_pid = clone(Thread::_trampoline, stack_top, flags, ptr);
	if (m_pid == -1) {
		toki::println("Can't create thread {}", errno);
	}
}

}  // namespace toki
