#pragma once

#include <toki/core.h>
#include <toki/renderer.h>

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

private:
	b32 m_is_running = false;
	RendererFrontend m_renderer;
	StaticArray<WeakRef<Window>, PLATFORM_MAX_WINDOW_COUNT> m_windows;

public:
	struct Config {};
};

}  // namespace toki
