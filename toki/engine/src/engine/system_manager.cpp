#include "system_manager.h"

#include "systems/texture_system.h"

namespace toki {

SystemManager::SystemManager(StackAllocator* allocator, Renderer* renderer): m_renderer(renderer) {
    m_systems.texture_system = allocator->emplace<TextureSystem>(this, allocator);
    m_systems.font_system = allocator->emplace<FontSystem>(this, allocator);
}

SystemManager::~SystemManager() {}

}  // namespace toki
