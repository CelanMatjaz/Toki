#include "font_system.h"

#include <freetype/freetype.h>
#include <freetype/ftmodapi.h>

#include <algorithm>

#include "core/assert.h"
#include "core/logging.h"
#include "engine/system_manager.h"
#include "renderer/configs.h"
#include "renderer/renderer.h"

namespace toki {

constexpr auto ATLAS_WIDTH = 512U;
constexpr auto ATLAS_HEIGHT = 512U;
constexpr auto START_GLYPH = 32U;
constexpr auto END_GLYPH = 127U;

FontSystem::FontSystem(SystemManager* system_manager, StackAllocator* allocator):
    m_systemManager(system_manager),
    m_fontMap(10, allocator) {
    allocator->allocate_aligned(sizeof(Font), alignof(Font));
}

FontSystem::~FontSystem() {}

const Font* FontSystem::get_font(std::string_view name) const {
    if (m_fontMap.contains(name)) {
        return &m_fontMap[name].value;
    }

    return nullptr;
}

void FontSystem::load_font(std::string_view name, std::string_view path, u32 font_size) {
    FT_Library library{};
    FT_Init_FreeType(&library);

    FT_Face face{};
    TK_ASSERT(FT_New_Face(library, std::string(path).c_str(), 0, &face) == 0, "Could not load font file");
    TK_LOG_INFO("Loaded font file '{}'", path);

    FT_Set_Pixel_Sizes(face, 0, font_size);

    Font font{};
    font.line_height = std::max((u32) ((face->size->metrics.ascender - face->size->metrics.descender) >> 6), font_size);

    std::vector<u8> atlas(ATLAS_WIDTH * ATLAS_HEIGHT);
    u32 col = 0;
    u32 row = 0;

    Glyph* glyphs = font.glyphs;

    for (u32 i = 0; i < END_GLYPH - START_GLYPH; i++) {
        TK_ASSERT(FT_Load_Char(face, i + START_GLYPH, FT_LOAD_RENDER) == 0, "Could not load char");

        FT_Bitmap& bitmap = face->glyph->bitmap;

        if (bitmap.width + col > ATLAS_WIDTH) {
            col = 0;
            row += font_size;
        }

        for (u32 glyph_y = 0; glyph_y < bitmap.rows; glyph_y++) {
            for (u32 glyph_x = 0; glyph_x < bitmap.width; glyph_x++) {
                atlas[((row + glyph_y) * ATLAS_WIDTH) + col + glyph_x] =
                    bitmap.buffer[glyph_y * bitmap.width + glyph_x];
            }
        }

        Glyph& glyph = glyphs[i];

        glyph.x = col;
        glyph.y = row;
        glyph.width = (u8) bitmap.width;
        glyph.height = (u8) bitmap.rows;
        glyph.advance_x = (u8) face->glyph->advance.x;
        glyph.advance_y = (u8) face->glyph->advance.y;
        glyph.offset_x = (u8) face->glyph->bitmap_left;
        glyph.offset_y = (u8) face->glyph->bitmap_top;

        col += bitmap.width;
    }

    FT_Done_Face(face);

    // Renderer& renderer = m_systemManager->get_renderer();
    // font.atlas_handle = renderer.create_texture(ColorFormat::R8, ATLAS_WIDTH, ATLAS_HEIGHT);
    // renderer.set_texture_data(font.atlas_handle, ATLAS_WIDTH * ATLAS_HEIGHT, atlas.data());

    FT_Done_Library(library);

    m_fontMap.emplace(name, font);

    TK_LOG_INFO("Loaded font '{}'", name);
}

}  // namespace toki
