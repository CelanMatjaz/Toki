#include "print.h"

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

}  // namespace toki
