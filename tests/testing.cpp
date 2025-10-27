#include "testing.h"

#include <toki/core/core.h>

int main() {
	using namespace toki;

	Allocator allocator(toki::MB(10));
	DefaultAllocator::allocator = &allocator;

	// toki::memory_initialize({ .total_size = toki::MB(100) });
	u32 passed = 0;
	u32 failed = 0;

	toki::println("==== Running {} test(s) ====", total_test_count);
	for (u32 i = 0; i < total_test_count; i++) {
		if (i == 25) {
			int a = 0;
		}
		TestCase& tc = test_cases[i];
		bool ok = tc.fn(tc);

		if (ok) {
			toki::print("{}[{}] PASSED ", green, i + 1);
			passed++;
		} else {
			toki::print("{}[{}] FAILED ", red, i + 1);
			failed++;
		}
		toki::print("[Test] {} - {}", tc.scope, tc.name);

		if (!ok) {
			toki::println("\n\t{}", tc.failed_file_line);
		}

		toki::println("{}", reset);
	}

	toki::println("\n==== Results ====");
	toki::println("passed: {}", passed);
	toki::println("failed: {}", failed);
}
