#include <toki/core/core.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/runtime.h>
#include <toki/sandbox/test_font_layer.h>
#include <toki/sandbox/test_layer.h>

using namespace toki;

toki::Mutex g_mutex;
u32 sum = 0;

void thread_func(u64 num) {
	for (u32 i = 0; i < num; i++) {
		ScopedLock lock(g_mutex);
		sum = sum + 1;
	};

	ScopedLock lock(g_mutex);
	toki::println("Ended thread, current sum: {}", sum);
}

void test_lambda_and_function_ptr() {
	u64 total_inc = 10101010;
	{
		// Lambda
		toki::Thread thread_lambda(
			[](u64 num) {
				thread_func(num);
			},
			total_inc);

		// Function pointer
		toki::Thread thread_function_ptr(thread_func, total_inc * 2);

		// toki::Function
		toki::Function<void(u64)> fn = thread_func;
		toki::Thread thread_toki_function(fn, total_inc * 4);
	}

	toki::println("sum: {}", sum);
}

i32 test_worker_functions_running = true;

void test_worker_functions() {
	toki::atomic_store(&test_worker_functions_running, true);

	ThreadPool thread_pool;
	thread_pool.add_worker(
		[](u64 count) {
			while (test_worker_functions_running) {
				sum += count;
			}
		},
		1);

	TK_LOG_DEBUG("Sum: {}", sum);
	u64 ms = 2000;
	TK_LOG_DEBUG("Waiting for {}ms", ms);
	auto _ = sleep(ms);
	toki::atomic_store(&test_worker_functions_running, false);
	toki::atomic_notify_all(&test_worker_functions_running);
	TK_LOG_DEBUG("Sum: {}", sum);
}

toki::i32 toki::toki_entrypoint([[maybe_unused]] toki::Span<char*> _) {
	// test_lambda_and_function_ptr();
	test_worker_functions();

	return 0;
}
