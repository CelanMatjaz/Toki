#pragma once

#include <toki/core/core.h>
#include <toki/renderer/types.h>
#include <vulkan/vulkan.h>

namespace toki::renderer {

struct ShaderHandle : public Handle {};
struct ShaderLayoutHandle : public Handle {};
struct BufferHandle : public Handle {};
struct TextureHandle : public Handle {};
struct SamplerHandle : public Handle {};

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

enum ShaderStageFlags : u8 {
	SHADER_STAGE_VERTEX = 1 << 0,
	SHADER_STAGE_FRAGMENT = 1 << 1,
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
	ShaderStageFlags stage;
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

enum struct SamplerFilter {
	NEAREST,
	LINEAR
};

enum struct SamplerAddressMode {
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_CLAMP_TO_EDGE
};

enum struct UniformType {
	UNIFORM_BUFFER,
	TEXTURE,
	SAMPLER,
	TEXTURE_WITH_SAMPLER
};

struct UniformConfig {
	u32 count = 1;
	u32 binding;
	UniformType type;
	ShaderStageFlags shader_stage_flags;
};

struct UniformSetConfig {
	Span<UniformConfig> uniform_configs;
};

struct BufferConfig {
	u64 size;
	BufferType type;
};

struct TextureConfig {
	u64 width;
	u64 height;
};

struct SamplerConfig {
	SamplerFilter mag_filter = SamplerFilter::LINEAR;
	SamplerFilter min_filter = SamplerFilter::LINEAR;
	SamplerAddressMode address_mode_u = SamplerAddressMode::CLAMP_TO_BORDER;
	SamplerAddressMode address_mode_v = SamplerAddressMode::CLAMP_TO_BORDER;
	SamplerAddressMode address_mode_w = SamplerAddressMode::CLAMP_TO_BORDER;
	b8 use_normalized_coords = true;
};

struct ShaderLayoutConfig {
	Span<UniformSetConfig> uniform_sets;
};

struct ShaderConfig {
	ShaderType type;
	ShaderOptions options;
	Array<StringView, SHADER_STAGE_SIZE> sources;
	Span<VertexBindingDescription> bindings;
	Span<VertexAttributeDescription> attributes;
	Span<ColorFormat> color_formats;
	Optional<ColorFormat> depth_format;
	ShaderLayoutHandle layout_handle;
};

struct SetUniform {
	union {
		BufferHandle uniform_buffer;
		TextureHandle texture;
		SamplerHandle sampler;
		struct {
			TextureHandle texture;
			SamplerHandle sampler;
		} texture_with_sampler;
	} handle;
	UniformType type : 4;
	u32 binding : 20;
	u32 array_element : 8;
	u32 set_index;
};

struct SetUniformConfig {
	ShaderLayoutHandle layout;
	Span<SetUniform> uniforms;
};

}  // namespace toki::renderer
