#include <toki/core/core.h>
#include <toki/runtime/runtime.h>

toki::i32 toki_entrypoint(toki::Span<char*> args) {
	toki::EngineConfig engine_config{};
	toki::Engine engine(engine_config);

	return 0;
}
