#include "print.h"

#include <cstdio>

#include "platform/platform.h"

namespace toki {

void print(const StringView& view) {
	using pt::write;
	write(pt::STD_OUT, view.data(), view.length());
}

void println(const StringView& view) {
	using pt::write;
	write(pt::STD_OUT, view.data(), view.length());
	write(pt::STD_OUT, "\n", 1);
}

void print_i64(i64 value, u32 radix) {
	char buf[20]{};
	u32 offset = 0;
	bool sign = value > -1;

	if (value == 0) {
		buf[offset++] = '0';
	} else {
		while (value > 0) {
			buf[offset++] = value % radix + '0';
			value /= radix;
		}
	}

	if (!sign) {
		buf[offset++] = '-';
	}

	char buf_out[20]{};
	for (u32 i = 0; i < offset; i++) {
		buf_out[i] = buf[offset - i - 1];
	}

	println(buf_out);
}

}  // namespace toki
