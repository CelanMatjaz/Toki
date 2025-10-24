#pragma once

#define STRINGIFY(x) #x
#define AS_STRING(x) STRINGIFY(x)

#if TK_TESTING_ENABLED
	#define private public
#else
	#define private private
#endif

#include <toki/core/core.h>

#include <csignal>

constexpr const char* reset = "\033[0m";
constexpr const char* bold = "\033[1m";
constexpr const char* red = "\033[31m";
constexpr const char* green = "\033[32m";

struct TestCase {
	const char* scope;
	const char* name;
	bool (*fn)(TestCase& tc);
	const char* failed_file_line{};
};

constexpr toki::u32 MAX_TESTS = 1024;
inline toki::u32 total_test_count = 0;
inline toki::Array<TestCase, MAX_TESTS> test_cases;

#define TK_TEST(scope, name)                                                                        \
	inline bool tk_test_##scope##_##name##_fn(TestCase& tc);                                        \
	struct tk_test_##scope##_##name##_register {                                                    \
		constexpr tk_test_##scope##_##name##_register() {                                           \
			test_cases[total_test_count++] = TestCase(#scope, #name, tk_test_##scope##_##name##_fn, ""); \
		}                                                                                           \
	} static inline tk_test_##scope##_##name##_register_instance;                                   \
	inline bool tk_test_##scope##_##name##_fn([[maybe_unused]] TestCase& tc)

#define TK_TEST_ASSERT(condition)                                   \
	{                                                               \
		if (!(condition)) {                                         \
			tc.failed_file_line = __FILE__ ":" AS_STRING(__LINE__); \
			std::raise(5);                                          \
			return false;                                           \
		}                                                           \
	}
