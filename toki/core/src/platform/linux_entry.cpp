#include "core/types.h"
#include "platform/platform.h"

extern int main();

extern "C" void _start() {
	toki::i32 main_result = main();
	toki::pt::exit(main_result);
}

extern "C" void __stack_chk_fail(void) {
	toki::pt::write(toki::pt::STD_ERR, "Stack overflow, the site\n", 26);
	toki::pt::exit(1);
}
