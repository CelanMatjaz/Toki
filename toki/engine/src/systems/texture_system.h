#pragma once

#include <utility>

#include "containers/hash_map.h"
#include "memory/allocators/stack_allocator.h"
#include "renderer/renderer_types.h"

namespace toki {

class SystemManager;

class TextureSystem {
	friend SystemManager;

public:
	TextureSystem(SystemManager* system_manager, StackAllocator* allocator);
	~TextureSystem();

public:
	const Texture* get_texture(std::string_view name) const;

	void create_texture(std::string_view name, u32 width, u32 height, ColorFormat format);
	void create_texture(std::string_view name, std::string_view path, ColorFormat format);

	void resize_texture(std::string_view name, u32 width, u32 height);
	void set_texture_data(std::string_view name, u32 size, void* data);

private:
	void create_default_textures();
	void cleanup_default_textures();

	enum class DefaultTextures : u8 {
		Default,
		SIZE
	};

	SystemManager* m_systemManager{};
	containers::HashMap<Texture> m_textureMap;
	Texture m_defaultTextures[std::to_underlying(DefaultTextures::SIZE)]{};
};

}  // namespace toki
