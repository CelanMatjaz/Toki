#include <toki/core.h>
#include <toki/engine.h>

int main() {
	toki::memory_initialize({ .total_size = toki::GB(1) });

	toki::Allocator alloc(toki::MB(64));
	toki::platform_initialize();

	toki::Engine::Config engine_config{};
	toki::Engine engine(engine_config);

	toki::Window::Config window_config{};
	window_config.title = "Test";
	window_config.width = 600;
	window_config.height = 600;
	engine.window_add(window_config);

	engine.run();

	toki::platform_shutdown();
}
