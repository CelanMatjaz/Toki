#include <toki/core.h>
#include <toki/engine.h>

int main() {
	toki::Allocator allocator(toki::MB(64));

	toki::Engine::Config engine_config{};
	toki::Engine engine(engine_config);
	engine.run();
}
