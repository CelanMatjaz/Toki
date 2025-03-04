#pragma once

#include "containers/hash_map.h"
#include "core/base.h"
#include "memory/allocators/stack_allocator.h"

namespace toki {

struct Glyph {
    u16 x, y;
    u8 codepoint;
    u8 width, height;
    u8 bearing_x, bearing_y;
    u8 advance_x, advance_y;
    u8 offset_x, offset_y;
};

struct Font {
    u32 line_height;
    Handle atlas_handle;
    Glyph glyphs[128];
};

class SystemManager;

class FontSystem {
    friend SystemManager;

public:
    FontSystem(SystemManager* system_manager, StackAllocator* allocator);
    ~FontSystem();

public:
    const Font* get_font(std::string_view name) const;

    void load_font(std::string_view name, std::string_view path, u32 font_size);

private:
    SystemManager* m_systemManager;
    containers::HashMap<Font> m_fontMap;
};

}  // namespace toki
