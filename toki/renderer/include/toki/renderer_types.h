#pragma once

#include <toki/core.h>

namespace toki {

struct Rect2D {
	Vec2<i32> pos;
	Vec2<u32> size;
};

constexpr u64 MAX_VERTEX_BINDING_COUNT = 4;
constexpr u64 MAX_VERTEX_ATTRIBUTE_COUNT = 8;

struct RendererObject {
	Handle handle;
};

struct Framebuffer : RendererObject {};
struct Buffer : RendererObject {};
struct Texture : RendererObject {};
struct Shader : RendererObject {};

enum ColorFormat : u8 {
	NONE,
	R8,
	RGBA8,
	DEPTH,
	STENCIL,

	COLOR_FORMAT_COUNT
};

enum class ShaderStage {
	VERTEX,
	FRAGMENT,

	SHADER_STAGE_COUNT
};

enum class VertexInputRate {
	VERTEX,
	INSTANCE,
};

enum class VertexFormat {
	FLOAT1,
	FLOAT2,
	FLOAT3,
	FLOAT4,
};

struct VertexAttributeDescription {
	u32 location;
	u32 binding;
	u32 offset;
	VertexFormat format;
};

struct VertexBindingDescription {
	u32 binding;
	u32 stride;
	VertexInputRate inputRate;
};

enum class BufferType {
	NONE,
	VERTEX,
	INDEX,
	UNIFORM,

	BUFFER_TYPE_COUNT
};

enum class CompareOp {
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	ALWAYS,
};

enum class PrimitiveTopology : u8 {
	POINT_LIST,
	LINE_LIST,
	LINE_STRIP,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	LINE_LIST_WITH_ADJACENCY,
	LINE_STRIP_WITH_ADJACENCY,
	TRIANGLE_LIST_WITH_ADJACENCY,
	TRIANGLE_STRIP_WITH_ADJACENCY,
	PATH_LIST,
};

enum class CullMode : u8 {
	NONE,
	FRONT,
	BACK,
	BOTH
};

enum class PolygonMode : u8 {
	FILL,
	LINE,
	POINT,
};

enum class FrontFace : u8 {
	COUNTER_CLOCKWISE,
	CLOCKWISE,
};

enum class ShaderType : u8 {
	GRAPHICS
};

struct FramebufferConfig {
	ColorFormat color_format : 8;
	u8 color_format_count : 6;
	b8 has_depth_attachment : 1;
	b8 has_stencil_attachment : 1;
	u32 image_width;
	u32 image_height;
};

struct BufferConfig {
	BufferType type;
	u32 size;
};

struct TextureConfig {
	ColorFormat format;
	u32 width;
	u32 height;
};

struct DepthTestConfig {
	CompareOp compare_operation : 4;
	b8 write_enable : 1;
};

struct ShaderConfig {
	ShaderType type = ShaderType::GRAPHICS;
	DynamicArray<ColorFormat> attachment_color_formats;
};

struct ShaderVariantConfig {
	StringView source_paths[static_cast<u32>(ShaderStage::SHADER_STAGE_COUNT)];
	BasicRef<DepthTestConfig> depth_test_config;
	PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
	FrontFace front_face : 1 = FrontFace::COUNTER_CLOCKWISE;
	CullMode cull_mode : 2 = CullMode::NONE;
	PolygonMode polygon_mode : 2 = PolygonMode::FILL;
	VertexBindingDescription vertex_bindings[MAX_VERTEX_BINDING_COUNT];
	VertexAttributeDescription vertex_attributes[MAX_VERTEX_ATTRIBUTE_COUNT];
	u32 binding_count;
	u32 attribute_count;
};

}  // namespace toki
