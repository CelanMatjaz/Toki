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
		TestCase& tc = test_cases[i];
		toki::print("[{}] ", i + 1);

		bool ok		 = tc.fn(tc);
		if (ok) {
			toki::print("{}PASSED", green);
			passed++;
		} else {
			toki::print("{}FAILED", red);
			failed++;
		}
		toki::println("{} {} {}", reset, tc.scope, tc.name);

		if (!ok) {
			toki::println("\t{}", tc.failed_file_line);
		}
	}

	toki::println("\n==== Results ====");
	toki::println("passed: {}", passed);
	toki::println("failed: {}", failed);
}
