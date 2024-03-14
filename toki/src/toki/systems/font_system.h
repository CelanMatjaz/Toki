#pragma once

#include "toki/renderer/texture.h"
#include "toki/resources/resource.h"

namespace Toki {

struct FontKerning {
    uint32_t codepoint0;
    uint32_t codepoint1;
    uint16_t amount;
};

struct GlyphData {
    uint32_t codepoint;
    float xOffset, yOffset;
    uint16_t x, y;
    uint16_t width, height;
    float xAdvance;
};

struct FontData {
    uint32_t size;
    uint32_t lineHeight;
    Ref<Texture> atlas;
    std::vector<GlyphData> glyphs;
    std::vector<FontKerning> kernings;
};

struct Font {
    const char* name;
    Resource resource;
    std::vector<uint8_t> binaryData;
    std::vector<Ref<FontData>> fontVersions;
};

class Application;

class FontSystem {
    friend Application;

public:
    static Ref<Font> loadBitmapFont(std::string name, const Resource& resource);

private:
    inline static Application* s_application = nullptr;

    static void init(Application* application);
    static void shutdown();
};

}  // namespace Toki
