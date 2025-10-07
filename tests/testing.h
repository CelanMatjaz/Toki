#pragma once

#define STRINGIFY(x) #x
#define AS_STRING(x) STRINGIFY(x)

#if TK_TESTING_ENABLED
	#define private public
#else
	#define private private
#endif



#include <string>
#include <csignal>
#include <vector>

constexpr const char* reset = "\033[0m";
constexpr const char* bold = "\033[1m";
constexpr const char* red = "\033[31m";
constexpr const char* green = "\033[32m";

struct TestCase {
	std::string scope;
	std::string name;
	bool (*fn)(TestCase& tc);
	std::string failed_file_line{};
};

inline std::vector<TestCase> test_cases;

#define TK_TEST(scope, name)                                                           \
	inline bool tk_test_##scope##_##name##_fn(TestCase& tc);                           \
	struct tk_test_##scope##_##name##_register {                                       \
		constexpr tk_test_##scope##_##name##_register() {                              \
			test_cases.emplace_back(#scope, #name, tk_test_##scope##_##name##_fn, ""); \
		}                                                                              \
	} static inline tk_test_##scope##_##name##_register_instance;                      \
	inline bool tk_test_##scope##_##name##_fn(TestCase& tc)

#define TK_TEST_ASSERT(condition)                                   \
	{                                                               \
		if (!(condition)) {                                         \
			tc.failed_file_line = __FILE__ ":" AS_STRING(__LINE__); \
			std::raise(5);                                    \
			return false;                                           \
		}                                                           \
	}
