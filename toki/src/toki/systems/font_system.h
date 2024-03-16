#pragma once

#include "toki/renderer/texture.h"
#include "toki/resources/resource.h"

namespace Toki {

struct FontKerning {
    uint32_t codepoint1;
    uint32_t codepoint2;
    uint16_t advance;
};

struct GlyphData {
    uint32_t codepoint;
    float xOffset, yOffset;
    uint16_t x, y;
    uint16_t width, height;
    float xAdvance;
};

struct FontData {
    uint16_t size;
    uint16_t lineHeight;
    uint16_t atlasWidth, atlasHeight;
    Ref<Texture> atlas;
    std::vector<GlyphData> glyphs;
    std::vector<FontKerning> kernings;
};

struct Font {
    std::string name;
    Resource resource;
    std::vector<uint8_t> binaryData;
    std::unordered_map<uint16_t, Ref<FontData>> fontVersions;
};

class Application;

class FontSystem {
    friend Application;

public:
    static void loadFont(std::string name, const Resource& resource);
    static void unloadFont(std::string name);
    static Ref<FontData> getFont(std::string name, uint16_t size);

private:
    inline static Application* s_application = nullptr;

    static void init(Application* application);
    static void shutdown();
};

}  // namespace Toki
