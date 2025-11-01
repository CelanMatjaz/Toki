#pragma once

#include <toki/renderer/renderer.h>
#include <toki/runtime/render/geometry.h>
#include <toki/runtime/render/types.h>

namespace toki {

struct Glyph {
	u16 x0, y0, x1, y1;
	f32 xoffset, yoffset, xadvance;
};

struct Font {
	TextureHandle atlas_handle;
	HashMap<u32, Glyph> glyph_data;
};

struct LoadFontConfig {
	Renderer* renderer;
	StringView path;
	Vector2u32 atlas_size;
	f32 size;
};

class FontSystem {
public:
	void load_font(toki::StringView name, const LoadFontConfig& config);
	ConstWrapper<Font> get_font(toki::StringView name);

	toki::Geometry generate_geometry(toki::StringView name, toki::StringView text);

private:
	toki::HashMap<toki::StringView, Font> m_fontMap{ 16 };
};

}  // namespace toki
