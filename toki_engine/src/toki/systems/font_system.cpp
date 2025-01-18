#include "font_system.h"

#include "core/assert.h"
#include "core/logging.h"
#include "freetype/freetype.h"
#include "freetype/ftmodapi.h"

namespace toki {

constexpr u32 MINIMUM_FONT_SIZE = 8;

constexpr u32 ATLAS_WIDTH = 512;
constexpr u32 ATLAS_HEIGHT = 512;

void FontSystem::initialize(const Config& config) {
    TK_LOG_INFO("Initializing font system...");
    TK_ASSERT(FT_Init_FreeType(&m_ft) == 0, "Could not initialize FreeType");
    TK_LOG_INFO("Font system initialized");
}

void FontSystem::shutdown() {
    FT_Done_Library(m_ft);
}

void FontSystem::load_font(const LoadFontConfig& config) {
    TK_ASSERT(config.font_size >= MINIMUM_FONT_SIZE, "Provided font size is not at minimum {}", MINIMUM_FONT_SIZE);

    FT_Face face;
    TK_ASSERT(FT_New_Face(m_ft, config.path.c_str(), 0, &face) == 0, "Could not load font file");
    TK_LOG_INFO("Loaded font file {}", config.path);

    TK_LOG_INFO("Glyph count: {}", face->num_glyphs);

    FT_Set_Pixel_Sizes(face, 0, config.font_size);

    std::vector<u8> atlas(ATLAS_WIDTH * ATLAS_HEIGHT);
    u32 col = 0;
    u32 row = 0;

    std::vector<Glyph> glyphs;

#define START_GLYPH 32
#define END_GLYPH 127

    glyphs.resize(128);

    for (u32 i = 0; i < END_GLYPH - START_GLYPH; i++) {
        TK_ASSERT(FT_Load_Glyph(face, i + START_GLYPH, FT_LOAD_RENDER) == 0, "Could not load char");
        TK_ASSERT(FT_Load_Char(face, i + START_GLYPH, FT_LOAD_RENDER) == 0, "Could not load char");

        FT_Bitmap& bitmap = face->glyph->bitmap;

        if (bitmap.width + col > ATLAS_WIDTH) {
            col = 0;
            row += config.font_size;
        }

        // Copy glyph to atlas
        for (u32 glyph_y = 0; glyph_y < bitmap.rows; glyph_y++) {
            for (u32 glyph_x = 0; glyph_x < bitmap.width; glyph_x++) {
                atlas[((row + glyph_y) * ATLAS_WIDTH) + col + glyph_x] = bitmap.buffer[glyph_y * bitmap.width + glyph_x];
            }
        }
       
        Glyph& glyph = glyphs[i];

        glyph.atlas_coords = { col, row };
        glyph.size = { (float) bitmap.width, (float) bitmap.rows };
        glyph.advance = { face->glyph->advance.x >> 6, face->glyph->advance.y >> 6 };
        glyph.offset = { face->glyph->bitmap_left, face->glyph->bitmap_top };

        col += bitmap.width;
    }

    FT_Done_Face(face);
}

}  // namespace toki
