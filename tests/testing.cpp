#include "testing.h"

#include <toki/core/core.h>

int main() {
	using namespace toki;

	toki::memory_initialize({ .total_size = toki::MB(10) });
	u32 passed = 0;
	u32 failed = 0;

	toki::println("==== Running {} test(s) ====", test_cases.size());
	for (u32 i = 0; i < test_cases.size(); i++) {
		TestCase& tc = test_cases[i];
		bool ok = tc.fn(tc);

		if (ok) {
			toki::print("{}[{}] PASSED ", green, i + 1);
			passed++;
		} else {
			toki::print("{}[{}] FAILED ", red, i + 1);
			failed++;
		}
		toki::print("[Test] {} - {}", tc.scope.data(), tc.name.data());

		if (!ok) {
			toki::println("\n\t{}", tc.failed_file_line.data());
		}

		toki::println("{}", reset);
	}

	toki::println("\n==== Results ====");
	toki::println("passed: {}", passed);
	toki::println("failed: {}", failed);
}
