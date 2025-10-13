#include "testing.h"
//

#include <toki/core/core.h>

using namespace toki;

TK_TEST(Converters, ftoa) {
	f64 value1 = 123456789.123456789;
	char buf[256]{};
	u32 length = toki::ftoa(buf, value1, 6);
	TK_TEST_ASSERT(toki::strncmp(buf, "123456789.123456", length));

	return true;
}
