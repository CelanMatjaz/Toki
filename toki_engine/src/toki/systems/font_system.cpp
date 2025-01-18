#include "font_system.h"

#include "core/assert.h"
#include "core/logging.h"
#include "freetype/freetype.h"

namespace toki {

FontSystem::FontSystem(const Config& config) {
    FT_Library ft;
    TK_ASSERT(FT_Init_FreeType(&ft) == 0, "Could not initialize FreeType");

    TK_LOG_INFO("Initializing font system");

    const char* font = "assets/fonts/Roboto-Regular.ttf";
    FT_Face face;
    TK_ASSERT(FT_New_Face(ft, font, 0, &face) == 0, "Could not load font file");
    TK_LOG_INFO("Loaded font {}", font);
}

FontSystem::~FontSystem() {}

}  // namespace toki
