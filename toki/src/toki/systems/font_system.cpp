#include "font_system.h"

#include <stb_truetype.h>

#include <print>
#include <unordered_map>

#include "toki/core/assert.h"
#include "toki/renderer/texture.h"
#include "toki/resources/loaders/binary_loader.h"

#define DEFAULT_FONT_ATLAS_WIDTH 1024
#define DEFAULT_FONT_ATLAS_HEIGHT 1024

namespace Toki {

struct FontSystemState {
    bool initialized = false;

    Application* application;

    std::unordered_map<std::string, Ref<Font>> fonts;
};

static FontSystemState* state = nullptr;

void FontSystem::init(Application* application) {
    TK_ASSERT(state == nullptr, "Font system state is already initialized");
    TK_ASSERT(application != nullptr, "Cannot init UISystem without a provided Application pointer");

    state = new FontSystemState;
    state->initialized = true;
    state->application = application;
}

void FontSystem::shutdown() {
    TK_ASSERT(state != nullptr && state->initialized, "Font system state is not initialized");

    for (auto& [name, font] : state->fonts) {
        font.reset();
    }

    delete state;
}

void FontSystem::loadFont(std::string name, const Resource& resource) {
    if (!state->fonts.contains(name)) {
        state->fonts[name] = createRef<Font>();

        auto fontBinary = BinaryLoader::readBinaryFile(resource.getPath());
        if (!fontBinary.has_value()) {
            return;
        }

        state->fonts[name]->name = name;
        state->fonts[name]->binaryData = fontBinary.value();
        state->fonts[name]->resource = resource;
    }
}

void FontSystem::unloadFont(std::string name) {
    state->fonts.erase(name);
}

Ref<FontData> FontSystem::getFont(std::string name, uint16_t size) {
    TK_ASSERT(state->fonts.contains(name), std::format("Font \"{}\" not loaded", name));

    if (state->fonts[name]->fontVersions.contains(size)) {
        return state->fonts[name]->fontVersions[size];
    }

    Ref<FontData> fontVersion = createRef<FontData>();

    uint32_t index = stbtt_GetFontOffsetForIndex(state->fonts[name]->binaryData.data(), 0);
    stbtt_fontinfo fontInfo{};
    int res = stbtt_InitFont(&fontInfo, state->fonts[name]->binaryData.data(), index);

    uint16_t atlasWidth = DEFAULT_FONT_ATLAS_WIDTH;
    uint16_t atlasHeight = DEFAULT_FONT_ATLAS_HEIGHT;

    fontVersion->atlasWidth = atlasWidth;
    fontVersion->atlasHeight = atlasHeight;

    TextureConfig atlasConfig{};
    atlasConfig.width = atlasWidth;
    atlasConfig.height = atlasHeight;
    Ref<Texture> atlas = Texture::create(atlasConfig);

    std::vector<int32_t> codePoints(96);
    codePoints[0] = -1;
    for (uint32_t i = 0; i < 95; ++i) {
        codePoints[i + 1] = i + 32;
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, size);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &line_gap);
    fontVersion->lineHeight = (ascent - descent + line_gap) * scale;

    std::vector<uint8_t> pixels(atlasWidth * atlasHeight);
    stbtt_pack_context context;
    TK_ASSERT(stbtt_PackBegin(&context, pixels.data(), atlasWidth, atlasHeight, 0, 1, 0), "");

    std::vector<stbtt_packedchar> charData(codePoints.size());

    stbtt_pack_range range;
    range.first_unicode_codepoint_in_range = 0;
    range.font_size = size;
    range.num_chars = codePoints.size();
    range.chardata_for_range = charData.data();
    range.array_of_unicode_codepoints = codePoints.data();

    TK_ASSERT(stbtt_PackFontRanges(&context, state->fonts[name]->binaryData.data(), 0, &range, 1), "");

    stbtt_PackEnd(&context);

    std::vector<uint8_t> rgbaPixels(atlasWidth * atlasHeight * 4);
    for (uint32_t j = 0; j < atlasWidth * atlasHeight; ++j) {
        rgbaPixels[(j * 4) + 0] = pixels[j];
        rgbaPixels[(j * 4) + 1] = pixels[j];
        rgbaPixels[(j * 4) + 2] = pixels[j];
        rgbaPixels[(j * 4) + 3] = pixels[j];
    }

    atlas->setData(rgbaPixels.size(), rgbaPixels.data());

    uint32_t kerningCount = stbtt_GetKerningTableLength(&fontInfo);
    std::vector<FontKerning> kernings(kerningCount);
    std::vector<stbtt_kerningentry> table(kerningCount);

    stbtt_GetKerningTable(&fontInfo, table.data(), table.size());

    for (uint32_t i = 0; i < table.size(); ++i) {
        kernings[i].codepoint1 = table[i].glyph1;
        kernings[i].codepoint2 = table[i].glyph2;
        kernings[i].advance = table[i].advance;
    }

    std::vector<GlyphData> glyphData(codePoints.size());
    for (uint32_t i = 0; i < codePoints.size(); ++i) {
        stbtt_packedchar* pc = &charData[i];
        glyphData[i].codepoint = codePoints[i];
        glyphData[i].xOffset = pc->xoff;
        glyphData[i].yOffset = pc->yoff;
        glyphData[i].x = pc->x0;
        glyphData[i].y = pc->y0;
        glyphData[i].width = pc->x1 - pc->x0;
        glyphData[i].height = pc->y1 - pc->y0;
        glyphData[i].xAdvance = pc->xadvance;
    }

    fontVersion->atlas = atlas;
    fontVersion->glyphs = std::move(glyphData);
    fontVersion->kernings = std::move(kernings);
    state->fonts[name]->fontVersions.emplace(size, fontVersion);

    return state->fonts[name]->fontVersions[size];
}

}  // namespace Toki
