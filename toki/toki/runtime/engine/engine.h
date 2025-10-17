#pragma once

#include <toki/core/core.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/engine/layer.h>

namespace toki {

struct EngineConfig {};

class Engine {
public:
	Engine() = delete;
	Engine(const EngineConfig& config);
	~Engine();

	void run();

	void attach_layer(UniquePtr<Layer>&& layer);

private:
	void cleanup();

	toki::UniquePtr<renderer::Renderer> m_renderer{};
	toki::UniquePtr<Window> m_window{};
	toki::b32 m_running{};

	DynamicArray<UniquePtr<Layer>> m_layers;
};

}  // namespace toki
