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
}

toki::i32 toki::toki_entrypoint([[maybe_unused]] toki::Span<char*> _) {
	u64 total_inc = 10000000;
	{
		toki::Thread thread_lambda(
			[](u64 num) {
				thread_func(num);
			},
			total_inc);
		toki::Thread thread_function_ptr(thread_func, 10000 + total_inc);

		thread_lambda.join();
		thread_function_ptr.join();
	}

	toki::println("sum: {}", sum);

	return 0;
}
