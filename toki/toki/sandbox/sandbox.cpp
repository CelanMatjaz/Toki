#include <toki/core/common/log.h>
#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/runtime.h>
#include <toki/sandbox/test_layer.h>

int main() {
	toki::platform::window_system_initialize();
	toki::memory_initialize({ .total_size = toki::GB(1) });

	{
		toki::runtime::EngineConfig runtime_config{};
		toki::runtime::Engine engine(runtime_config);

		engine.attach_layer(toki::move(toki::make_unique<TestLayer>(0.0f)));

		engine.run();
	}

	toki::memory_shutdown();
	toki::platform::window_system_shutdown();
}
