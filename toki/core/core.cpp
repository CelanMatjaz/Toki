#include "core/log.h"
#include "core/macros.h"
#include "math/math.h"
#include "platform/defines.h"
#include "platform/platform.h"
#include "core/assert.h"
#include "print.h"

int abs(int value) {
	return value >= 0 ? value : -value;
}

#define CHECK_ERROR(value)                 \
	if (value < 0) {                       \
		return toki::pt::get_last_error(); \
	}

void print_uint(toki::pt::Handle handle, toki::u64 value) {
	char buf[20];
	toki::u32 buf_offset = 0;

	char digit{};
	while (value > 0) {
		digit = value % 10 + '0';
		value = value / 10;
		buf[buf_offset++] = digit;
	}

	char buf_out[20]{};
	for (toki::u32 i = 0; i < buf_offset; i++) {
		buf_out[i] = buf[buf_offset - i - 1];
	}

	buf_out[buf_offset] = '\n';
	toki::pt::write(handle, buf_out, buf_offset + 1);
}



int main() {

    

    constexpr auto result = toki::pow(3, 2);
    toki::println(AS_STRING(result));
    
}
