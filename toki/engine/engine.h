#pragma once

#include <toki/core.h>

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

	void window_add(const char* title, u32 width, u32 height);

private:
	// Allocator mEngineAllocator;
	b32 m_is_running = false;

public:
	struct Config {};
};

}  // namespace toki
