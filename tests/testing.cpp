#include "testing.h"

#include <print>

int main() {
	uint32_t passed = 0;
	uint32_t failed = 0;

	std::println("==== Running {} test(s) ====", test_cases.size());
	for (uint32_t i = 0; i < test_cases.size(); i++) {
		TestCase& tc = test_cases[i];
		bool ok = tc.fn(tc);

		if (ok) {
			std::print("{}[{}] PASSED ", green, i + 1);
			passed++;
		} else {
			std::print("{}[{}] FAILED ", red, i + 1);
			failed++;
		}
		std::print("[Test] {} - {}", tc.scope, tc.name);

		if (!ok) {
			std::println("\n\t{}", tc.failed_file_line);
		}

		std::println("{}", reset);
	}

	std::println("\n==== Results ====");
	std::println("passed: {}", passed);
	std::println("failed: {}", failed);
}
