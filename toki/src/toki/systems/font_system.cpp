#include "font_system.h"

#include <stb_truetype.h>

#include <print>
#include <unordered_map>

#include "toki/core/assert.h"
#include "toki/renderer/texture.h"
#include "toki/resources/loaders/binary_loader.h"

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

Ref<Font> FontSystem::loadBitmapFont(std::string name, const Resource& resource) {
    if (state->fonts.contains(name)) {
        return state->fonts[name];
    }

    // TK_ASSERT(!state->fonts.contains(name), std::format("Font with name \"{}\" already exists", name));

    auto fontBinary = BinaryLoader::readBinaryFile(resource.getPath());
    Ref<Font> font = createRef<Font>();
    font->binaryData = fontBinary.value();

    auto f = createRef<FontData>();

    uint32_t index = stbtt_GetFontOffsetForIndex(fontBinary.value().data(), 0);
    stbtt_fontinfo fontInfo{};
    int res = stbtt_InitFont(&fontInfo, fontBinary.value().data(), index);

    TK_ASSERT(res != 0, "");

    TextureConfig atlasConfig{};
    atlasConfig.width = atlasConfig.height = 1024;
    Ref<Texture> atlas = Texture::create(atlasConfig);

    std::vector<int> codePoints(96);
    codePoints[0] = -1;
    for (uint32_t i = 0; i < 95; ++i) {
        codePoints[i + 1] = i + 32;
    }

    uint32_t fontsize = 20;

    int a, b, c, d;
    stbtt_GetFontBoundingBox(&fontInfo, &a, &b, &c, &d);
    std::println("{} {} {} {}", a, b, c, d);

    int scale = stbtt_ScaleForPixelHeight(&fontInfo, fontsize);
    std::println("scale {}", scale);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &line_gap);

    std::println("123 {} {} {}", ascent, descent, line_gap);

    f->lineHeight = (ascent - descent + line_gap) * scale;

    std::vector<uint8_t> pixels(1024 * 1024);
    stbtt_pack_context context;
    res = stbtt_PackBegin(&context, pixels.data(), 1024, 1024, 0, 1, 0);
    std::println("stbtt_PackBegin {}", res);

    stbtt_packedchar* charData = new stbtt_packedchar[codePoints.size()];

    stbtt_pack_range range;
    range.first_unicode_codepoint_in_range = 0;
    range.font_size = fontsize;
    range.num_chars = codePoints.size();
    range.chardata_for_range = charData;
    range.array_of_unicode_codepoints = codePoints.data();

    res = stbtt_PackFontRanges(&context, fontBinary.value().data(), 0, &range, 1);
    std::println("stbtt_PackFontRanges {}", res);

    stbtt_PackEnd(&context);

    std::vector<uint8_t> rgbaPixels(1024 * 1024 * 4);
    for (uint32_t j = 0; j < 1024 * 1024; ++j) {
        rgbaPixels[(j * 4) + 0] = pixels[j];
        rgbaPixels[(j * 4) + 1] = pixels[j];
        rgbaPixels[(j * 4) + 2] = pixels[j];
        rgbaPixels[(j * 4) + 3] = pixels[j];
    }

    atlas->setData(rgbaPixels.size(), rgbaPixels.data());

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

    std::println("count: {}", glyphData.size());

    uint32_t kerningCount = stbtt_GetKerningTableLength(&fontInfo);
    std::vector<FontKerning> kernings(kerningCount);
    std::vector<stbtt_kerningentry> table(kerningCount);

    stbtt_GetKerningTable(&fontInfo, table.data(), table.size());

    for (uint32_t i = 0; i < table.size(); ++i) {
        kernings[i].codepoint0 = table[i].glyph1;
        kernings[i].codepoint1 = table[i].glyph2;
        kernings[i].amount = table[i].advance;
    }

    std::println("kernings: {}", kernings.size());

    delete[] charData;

    f->glyphs = glyphData;

    f->atlas = atlas;
    font->fontVersions.emplace_back(f);

    state->fonts.emplace(name, font);

    return font;

    // std::vector<font_glyph> glyphs(codePoints.size());
}

}  // namespace Toki
