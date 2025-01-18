#pragma once

#include <freetype/freetype.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "core/base.h"

namespace toki {

struct Glyph {
    glm::ivec2 atlas_coords;
    glm::ivec2 size;
    glm::ivec2 bearing;
    glm::ivec2 advance;
    glm::ivec2 offset;
};

struct FontVariant {
    u32 size;
    std::vector<Glyph> glyphs;
};

struct Font {
    std::string_view path;
    std::unordered_map<u32, FontVariant> variants;
};

class FontSystem {
public:
    struct Config {
        u16 default_size;
    };

    struct LoadFontConfig {
        std::string path;
        u32 font_size;
    };

public:
    FontSystem() = default;
    ~FontSystem() = default;

    void initialize(const Config& config);
    void shutdown();

    void load_font(const LoadFontConfig& config);

private:
    FT_Library m_ft{};
};

}  // namespace toki
