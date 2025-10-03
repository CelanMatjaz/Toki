#pragma once

#include <toki/core/core.h>
#include <toki/renderer/types.h>
#include <vulkan/vulkan.h>

namespace toki::renderer {

struct ShaderHandle : public Handle {};

struct ShaderLayoutHandle : public Handle {};

enum struct ColorFormat : u16 {
	NONE,
	R8,
	RGBA8,
	// DEPTH,
	// STENCIL,
	// DEPTH_STENCIL,

	COLOR_FORMAT_COUNT
};

enum struct ShaderType : u8 {
	GRAPHICS
};

enum ShaderStage : u8 {
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_FRAGMENT,
	SHADER_STAGE_SIZE
};

enum struct VertexInputRate : u8 {
	VERTEX,
	INSTANCE,
};

enum struct VertexFormat : u8 {
	FLOAT1,
	FLOAT2,
	FLOAT3,
	FLOAT4,
};

struct VertexAttributeDescription {
	u32 location;
	u32 binding;
	VertexFormat format;
	u32 offset;
};

struct VertexBindingDescription {
	u32 binding;
	u32 stride;
	VertexInputRate inputRate;
};

enum struct BufferType : u8 {
	VERTEX,
	INDEX,
	UNIFORM,
	SIZE
};

enum struct BufferUsage : u8 {
	NONE,
	STATIC,
	DYNAMIC,
};

enum struct CompareOp : u8 {
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	ALWAYS,
};

enum struct PrimitiveTopology : u8 {
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
	PATCH_LIST,
};

enum struct CullMode : u8 {
	NONE,
	FRONT,
	BACK,
	BOTH
};

enum struct PolygonMode : u8 {
	FILL,
	LINE,
	POINT,
};

enum struct FrontFace : u8 {
	COUNTER_CLOCKWISE,
	CLOCKWISE,
};

struct ShaderSourceFile {
	ShaderStage stage;
	StringView path;
};

struct ShaderOptions {
	PrimitiveTopology primitive_topology : 4;
	CullMode cull_mode : 2;
	PolygonMode polygon_mode : 2;
	CompareOp depth_compare_op : 3;
	FrontFace front_face : 1;
	b8 depth_test_enable : 1;
	b8 depth_write_enable : 1;
};

struct ShaderBindingConfig {
	u32 binding;
};

struct ShaderLayoutConfig {};

struct ShaderConfig {
	ShaderType type;
	ShaderOptions options;
	TempStaticArray<StringView, SHADER_STAGE_SIZE> sources;
	Span<VertexBindingDescription> bindings;
	Span<VertexAttributeDescription> attributes;
	Span<ColorFormat> color_formats;
	Optional<ColorFormat> depth_format;
	ShaderLayoutHandle layout_handle;
};

}  // namespace toki::renderer
