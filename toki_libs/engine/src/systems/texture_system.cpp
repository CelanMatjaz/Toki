#include "texture_system.h"

#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/assert.h"
#include "core/logging.h"
#include "engine/system_manager.h"

#if 0

namespace toki {

TextureSystem::TextureSystem(SystemManager* system_manager, StackAllocator* allocator):
    m_systemManager(system_manager),
    m_textureMap(200, allocator) {}

TextureSystem::~TextureSystem() {
    cleanup_default_textures();

    for (auto& texture : m_textureMap) {
        m_systemManager->get_renderer().destroy_texture(texture.value.handle);
    }
}

const Texture* TextureSystem::get_texture(std::string_view name) const {
    if (m_textureMap.contains(name)) {
        return &m_textureMap[name].value;
    }

    return nullptr;
}

void TextureSystem::create_texture(std::string_view name, u32 width, u32 height, ColorFormat format) {
    TK_LOG_INFO("Creating texture '{}'", name);

    TextureCreateConfig texture_create_config{};
    texture_create_config.size = { width, height };
    texture_create_config.format = format;
    Handle texture_handle = m_systemManager->get_renderer().create_texture(texture_create_config);

    m_textureMap.emplace(std::string{ name }, width, height, format, texture_handle);
}

void TextureSystem::create_texture(std::string_view name, std::string_view path, ColorFormat format) {
    TK_LOG_INFO("Creating texture '{}' from file '{}'", name, path);
    Renderer& renderer = m_systemManager->get_renderer();

    int width, height, channels;
    int desired_channels = STBI_rgb_alpha;

    switch (format) {
        case ColorFormat::R8:
            desired_channels = STBI_grey;
            break;
        default:
            desired_channels = STBI_rgb_alpha;
    }

    std::filesystem::path file_path(path);
    u32* pixels = (uint32_t*) stbi_load(file_path.string().c_str(), &width, &height, &channels, desired_channels);
    TK_ASSERT(pixels != nullptr, "Error loading image");
    u64 image_size = width * height * desired_channels;
    TK_ASSERT(image_size > 0, "Image size is 0");

    TextureCreateConfig config{};
    config.format = format;
    config.size = { width, height };
    Handle texture_handle = renderer.create_texture(config);
    renderer.set_texture_data(texture_handle, image_size, pixels);
    stbi_image_free(pixels);
    m_textureMap.emplace(std::string{ name }, width, height, format, texture_handle);
}

void TextureSystem::resize_texture(std::string_view name, u32 width, u32 height) {
    TK_ASSERT(m_textureMap.contains(name), "Texture with name '{}' does not exist", name);
}

void TextureSystem::set_texture_data(std::string_view name, u32 size, void* data) {
    TK_ASSERT(m_textureMap.contains(name), "Texture with name '{}' does not exist", name);
    m_systemManager->get_renderer().set_texture_data(m_textureMap[name].value.handle, size, data);
}

void TextureSystem::create_default_textures() {
    Renderer& renderer = m_systemManager->get_renderer();

    {
        TextureCreateConfig default_texture_config{};
        default_texture_config.size = { 2, 2 };
        default_texture_config.format = ColorFormat::RGBA8;
        Handle default_texture_handle = renderer.create_texture(default_texture_config);

        uint32_t pixels[] = {
            0xFFFF00FF,
            0xFF000000,
            0xFF000000,
            0xFFFF00FF,
        };

        renderer.set_texture_data(default_texture_handle, sizeof(pixels), pixels);

        m_defaultTextures[std::to_underlying(DefaultTextures::Default)].handle = { default_texture_handle };
    }
}

void TextureSystem::cleanup_default_textures() {
    Renderer& renderer = m_systemManager->get_renderer();

    renderer.destroy_texture(m_defaultTextures[std::to_underlying(DefaultTextures::Default)].handle);
}


}  // namespace toki

#endif
