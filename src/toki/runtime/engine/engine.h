#pragma once

#include <toki/core/core.h>
#include <toki/renderer/renderer.h>
#include <toki/runtime/engine/layer.h>

#include "toki/runtime/systems/system_manager.h"

namespace toki {

struct EngineConfig {};

class Engine {
public:
	Engine() = delete;
	Engine(const EngineConfig& config);
	~Engine();

	void run();
	void attach_layer(UniquePtr<Layer>&& layer);

	Renderer* renderer() const {
		return m_renderer.get();
	}

	Window* window() const {
		return m_window.get();
	}

	SystemManager* system_manager() const {
		return m_systemManager.get();
	}

private:
	void cleanup();

	toki::UniquePtr<Renderer> m_renderer{};
	toki::UniquePtr<Window> m_window{};
	toki::UniquePtr<SystemManager> m_systemManager{};
	toki::b32 m_running{};

	DynamicArray<UniquePtr<Layer>> m_layers;
};

}  // namespace toki
