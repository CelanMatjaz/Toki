#pragma once

#include <toki/core.h>
#include <toki/renderer.h>

#include "layer.h"

namespace toki {

class Engine {
public:
	struct Config;

public:
	Engine() = delete;
	Engine(const Config& config);
	~Engine();

	DELETE_COPY(Engine);
	DELETE_MOVE(Engine);

	void run();

	void window_add(const Window::Config& config);

	template <typename L>
	void layer_push() {
		m_layers.resize(m_layers.size() + 1);
		m_layers.last() = new L();
		m_layers.last()->m_renderer = &m_renderer;
		auto last = m_layers.last();
		m_layers[m_layers.size() - 1]->on_attach();
	}

private:
	b32 m_is_running = false;
	RendererFrontend m_renderer;
	DynamicArray<Layer*> m_layers;
	StaticArray<WeakRef<Window>, PLATFORM_MAX_WINDOW_COUNT> m_windows;

public:
	struct Config {};
};

}  // namespace toki
