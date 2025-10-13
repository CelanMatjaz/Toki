#include <toki/core/common/log.h>
#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/runtime.h>
#include <toki/sandbox/test_layer.h>

int main() {
	toki::platform::window_system_initialize();
	toki::memory_initialize({ .total_size = toki::GB(1) });

	toki::Matrix4 expected(
		2.0f,
		0.0f,
		0.0f,
		0.0f,  // column 0
		0.0f,
		2.0f,
		0.0f,
		0.0f,  // column 1
		0.0f,
		0.0f,
		2.0f,
		0.0f,  // column 2
		1.0f,
		2.0f,
		3.0f,
		1.0f  // column 3 (translation)
	);

	toki::println("printing");
	toki::println(toki::Formatter<toki::Matrix4>::format(expected));
	toki::println(toki::Formatter<toki::Matrix4>::format(expected));
	toki::println(toki::Formatter<toki::Matrix4>::format(expected));
	toki::println("testing }} {{ {{ }} printing {} {} {} TEST>>>", "this is a", 124.56, true);
	toki::println("<<<printing");

	// return 0;

	{
		toki::runtime::EngineConfig runtime_config{};
		toki::runtime::Engine engine(runtime_config);

		engine.attach_layer(toki::make_unique<TestLayer>(0.0f));
		engine.run();
	}

	toki::memory_shutdown();
	toki::platform::window_system_shutdown();
}
