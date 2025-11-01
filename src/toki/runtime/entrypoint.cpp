#include "toki/runtime/entrypoint.h"

toki::i32 main(int argc, char** argv) {
	toki::window_system_initialize();
	toki::memory_initialize({ .total_size = toki::GB(4) });

	toki::i32 result = toki_entrypoint(toki::Span(argv, argc));

	toki::memory_shutdown();
	toki::window_system_shutdown();

	return result;
}
