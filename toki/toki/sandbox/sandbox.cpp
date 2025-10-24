#include <toki/core/common/log.h>
#include <toki/core/core.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/runtime.h>
#include <toki/sandbox/test_layer.h>

int main() {
	toki::window_system_initialize();
	toki::memory_initialize({ .total_size = toki::GB(4) });

	// auto model_data = toki::load_obj("zajecMatjazTrikotniki.obj");

	{
		toki::EngineConfig runtime_config{};
		toki::Engine engine(runtime_config);

		engine.attach_layer(toki::make_unique<TestLayer>(1.0f));
		engine.run();
	}

	toki::memory_shutdown();
	toki::window_system_shutdown();
}
