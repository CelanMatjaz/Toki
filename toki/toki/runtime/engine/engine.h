#pragma once

#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/renderer/renderer.h>

namespace toki::runtime {

struct EngineConfig {};

class Engine {
public:
	Engine() = delete;
	Engine(const EngineConfig& config);
	~Engine();

	void run();

	const platform::Window* get_window(u32 index) const;

	static Engine* get() {
		return s_runtime;
	}

	static renderer::Renderer* renderer() {
		return get()->m_renderer.get();
	}

private:
	void cleanup();

	toki::UniquePtr<renderer::Renderer> m_renderer{};
	toki::UniquePtr<platform::Window> m_window{};
	toki::u64 m_frameCount{};
	toki::b32 m_running{};

	static inline Engine* s_runtime;
};

}  // namespace toki::runtime
