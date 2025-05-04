#pragma once

#include "memory/allocators/stack_allocator.h"
#include "renderer/renderer.h"
#include "systems/font_system.h"
#include "systems/texture_system.h"

namespace toki {

struct Systems {
	TextureSystem* texture_system;
	FontSystem* font_system;
};

class SystemManager {
	friend class Engine;

public:
	SystemManager() = delete;
	SystemManager(StackAllocator* stack_allocator, Renderer* renderer);
	~SystemManager();

public:
	Systems& get_systems() {
		return m_systems;
	}

	Renderer& get_renderer() {
		return *m_renderer;
	}

private:
	Systems m_systems;
	Renderer* m_renderer;
};

}  // namespace toki
