#include <toki/core/common/log.h>
#include <toki/core/core.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/runtime.h>
#include <toki/sandbox/test_font_layer.h>
#include <toki/sandbox/test_layer.h>

toki::i32 toki::toki_entrypoint([[maybe_unused]] toki::Span<char*> _) {
	toki::EngineConfig runtime_config{};
	toki::Engine engine(runtime_config);

	engine.attach_layer(toki::make_unique<TestFontLayer>());
	engine.attach_layer(toki::make_unique<TestLayer>());
	engine.run();

	return 0;
}
