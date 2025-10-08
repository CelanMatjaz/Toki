#pragma once

#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/engine/layer.h>

#include "toki/core/common/common.h"
#include "toki/core/memory/unique_ptr.h"

namespace toki::runtime {

struct EngineConfig {};

class Engine {
public:
	Engine() = delete;
	Engine(const EngineConfig& config);
	~Engine();

	void run();

	void attach_layer(UniquePtr<Layer>&& layer);

	const platform::Window* get_window(u32 index) const;

private:
	void cleanup();

	toki::UniquePtr<renderer::Renderer> m_renderer{};
	toki::UniquePtr<platform::Window> m_window{};
	toki::u64 m_frameCount{};
	toki::b32 m_running{};

	DynamicArray<UniquePtr<Layer>> m_layers;
};

}  // namespace toki::runtime
