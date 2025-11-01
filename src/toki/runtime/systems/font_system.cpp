#include "toki/runtime/systems/font_system.h"

#include <toki/core/core.h>

#define STBTT_ifloor(x)	   toki::floor(x)
#define STBTT_iceil(x)	   toki::ceil(x)
#define STBTT_sqrt(x)	   toki::sqrt(x)
#define STBTT_pow(x, y)	   toki::pow(x, y)
#define STBTT_fmod(x, y)   toki::mod<toki::f32>(x, y)
#define STBTT_cos(x)	   toki::cos(x)
#define STBTT_acos(x)	   toki::acos(x)
#define STBTT_fabs(x)	   toki::abs<toki::f32>(x)
#define STBTT_assert(x)	   toki::assert(x)
#define STBTT_strlen(x)	   toki::strlen(x)
#define STBTT_memcpy	   toki::memcpy
#define STBTT_memset	   toki::memset
#define STBTT_malloc(x, u) toki::DefaultAllocator::allocate(x)
#define STBTT_free(x, u)   toki::DefaultAllocator::free(x)

#define STB_TRUETYPE_IMPLEMENTATION

PUSH_WARNING;
DISABLE_UNUSED_PARAM;
#include <stb_truetype.h>
POP_WARNING;

namespace toki {

void FontSystem::load_font(toki::StringView name, const LoadFontConfig& config) {
	u32 first_char = 32;
	u32 char_count = 96;

	Font font{};

	File file(config.path);
	file.seek(0, FileCursorStart::END);
	u32 file_byte_count = file.tell();
	DynamicArray<u8> ttf_bytes(file_byte_count);
	file.seek(0, FileCursorStart::BEGIN);
	file.read(ttf_bytes.data(), file_byte_count);

	using PixelType = u8;
	u32 pixel_count = config.atlas_size.x * config.atlas_size.y;
	DynamicArray<PixelType> initial_pixels(pixel_count, 255);

	DynamicArray<Glyph> glyphs(char_count);

	i32 result = stbtt_BakeFontBitmap(
		ttf_bytes.data(),
		0,
		config.size,
		reinterpret_cast<u8*>(initial_pixels.data()),
		static_cast<i32>(config.atlas_size.x),
		static_cast<i32>(config.atlas_size.y),
		first_char,
		char_count,
		reinterpret_cast<stbtt_bakedchar*>(glyphs.data()));
	TK_ASSERT(result > 0);

	union Temp {
		u8 values[4];
		u32 v;
	};
	DynamicArray<Temp> pixels(pixel_count);
	for (u32 i = 0; i < pixel_count; i++) {
		pixels[i].values[0] = 100;
		pixels[i].values[1] = 220;
		pixels[i].values[2] = 200;
		pixels[i].values[3] = initial_pixels[i];
	}

	font.glyph_data.reset(char_count);

	for (u32 i = 0; i < glyphs.size(); i++) {
		font.glyph_data.emplace(i + first_char, glyphs[i]);
	}

	TextureConfig texture_config{};
	texture_config.channels = 2;
	texture_config.format	= ColorFormat::R8;
	texture_config.width	= config.atlas_size.x;
	texture_config.height	= config.atlas_size.y;
	texture_config.flags	= SAMPLED | COLOR_ATTACHMENT | WRITABLE;

	font.atlas_handle = config.renderer->create_texture(texture_config);

	config.renderer->set_texture_data(font.atlas_handle, initial_pixels.data(), pixels.size() * sizeof(char));

	TK_LOG_INFO("Creating [Font] \"{}\"", name);

	m_fontMap.emplace(name, toki::move(font.atlas_handle), toki::move(font.glyph_data));
}

ConstWrapper<Font> FontSystem::get_font(toki::StringView name) {
	TK_ASSERT(m_fontMap.contains(name));
	return m_fontMap.at(name);
}

toki::Geometry FontSystem::generate_geometry(toki::StringView name, toki::StringView text) {
	TK_ASSERT(m_fontMap.contains(name));
	const Font& font = m_fontMap.at(name);

	u32 vertex_data_size = sizeof(FontVertex) * 4 * text.size();
	FontVertex* vertices = reinterpret_cast<FontVertex*>(DefaultAllocator::allocate(vertex_data_size));

	toki::DynamicArray<u32> indices;
	indices.reserve(text.size() * 6);

	f32 cursor_x = 0;
	f32 cursor_y = 0;
	u32 index	 = 0;
	FontVertex v;

	constexpr const f32 atlas_width	 = 512;
	constexpr const f32 atlas_height = 512;

	for (u32 i = 0; i < text.size(); i++) {
		if (text[i] == ' ') {
			cursor_x += 10;
			continue;
		}

		Glyph& glyph = font.glyph_data.at(text[i]);

		f32 u0 = glyph.x0 / atlas_width;
		f32 v0 = (glyph.y0 / atlas_height);
		f32 u1 = glyph.x1 / atlas_width;
		f32 v1 = (glyph.y1 / atlas_height);

		f32 x0 = cursor_x + glyph.xoffset;
		f32 y0 = -(cursor_y + glyph.yoffset);
		f32 x1 = x0 + (glyph.x1 - glyph.x0);
		f32 y1 = -(y0 - (glyph.y1 - glyph.y0));

		vertices[index + 0] = { Vector3{ x0, y0, 0 }, Vector2{ u0, v0 } };
		vertices[index + 1] = { Vector3{ x1, y0, 0 }, Vector2{ u1, v0 } };
		vertices[index + 2] = { Vector3{ x0, y1, 0 }, Vector2{ u0, v1 } };
		vertices[index + 3] = { Vector3{ x1, y1, 0 }, Vector2{ u1, v1 } };

		indices.push_back(index + 0);
		indices.push_back(index + 1);
		indices.push_back(index + 2);
		indices.push_back(index + 2);
		indices.push_back(index + 1);
		indices.push_back(index + 3);

		cursor_x += glyph.xadvance;
		index += 4;
	}

	return { vertices, vertex_data_size, toki::move(indices) };
}

}  // namespace toki
