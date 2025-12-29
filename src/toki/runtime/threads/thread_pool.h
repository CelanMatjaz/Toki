#pragma once

#include <toki/core/core.h>
#include <toki/runtime/threads/worker_thread.h>

namespace toki {

class ThreadPool {
public:
	template <typename Callable, typename... Args>
		requires CIsCorrectCallable<Callable, void, Args...>
	void add_worker(Callable&& callable, Args&&... args) {
		WorkerThread worker{};
		worker.thread.start(callable, toki::forward<Args>(args)...);
		m_threads.emplace_back(toki::move(worker));
		TK_LOG_INFO("Added new worker thread");
	}

private:
	DynamicArray<WorkerThread> m_threads;
};

}  // namespace toki
