#pragma once

#include <toki/core/core.h>

namespace toki {

static constexpr const u32 MAX_WORKER_THREAD_QUEUE_ELEMENT_COUNT = 32;

struct WorkerThread {
	Thread thread;
	Mutex mutex;
	RingBuffer<i32, MAX_WORKER_THREAD_QUEUE_ELEMENT_COUNT> work_queue;
};

}  // namespace toki
