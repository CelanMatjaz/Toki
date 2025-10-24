#include "testing.h"
//

#include <toki/core/core.h>

using namespace toki;

TK_TEST(Converters, ftoa) {
	f64 value1 = 123456789.123456789;
	char buf[256]{};
	u32 length = toki::ftoa(buf, value1, 6);
	TK_TEST_ASSERT(toki::strncmp(buf, "123456789.123456", length) == 0);

	return true;
}

TK_TEST(Converters, itoa) {
	u64 value1 = 123456789;
	char buf[256]{};
	u32 length = toki::itoa(buf, value1, 10);
	TK_TEST_ASSERT(toki::strncmp(buf, "123456789", length) == 0);

	return true;
}

TK_TEST(Converters, atof) {
	const char buf[] = " 0.052997 -0.122533 0.050569";
	const char* temp = buf;
	f64 value_out{};
	f64 values[3]{};
	u32 buf_index = 0;
	for (u32 i = 0; i < 3; i++) {
		u32 converted_count = atof(temp, value_out);
		temp += converted_count - 1;
		values[i] = value_out;
	}

	i32 expected_values[3] = { 52997, -122533, 50569 };

	for (u32 i = 0; i < ARRAY_SIZE(values); i++) {
		f64 value = values[i];
		i32 actual_value = static_cast<i32>(value * 1000000);
		TK_ASSERT(actual_value == expected_values[i]);
	}

	return true;
}
