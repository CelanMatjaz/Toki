#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/runtime/runtime.h>

#include <toki/core/common/log.h>
#include <toki/renderer/renderer.h>

int main() {
	toki::platform::window_system_initialize();
	toki::memory_initialize({ .total_size = toki::GB(1) });

	toki::renderer::ShaderConfig shader_config{};
	shader_config.sources[toki::renderer::SHADER_STAGE_VERTEX] = "";
	shader_config.sources[toki::renderer::SHADER_STAGE_FRAGMENT] = "";

	TK_LOG_DEBUG("test");
	{
		toki::runtime::EngineConfig runtime_config{};
		toki::runtime::Engine engine(runtime_config);
		engine.run();
	}

	toki::memory_shutdown();
	toki::platform::window_system_shutdown();
}
