#pragma once

#include <toki/core/core.h>
#include <toki/core/math/vector2.h>
#include <toki/core/math/vector3.h>
#include <toki/renderer/renderer.h>

namespace toki {

struct Vertex {
	Vector3 position;
	Vector3 normals;
	Vector2 uv;

	b8 operator<=>(const Vertex&) const = default;

	static toki::Array<VertexBindingDescription, 1> vertex_bindings;
	static toki::Array<VertexAttributeDescription, 3> vertex_attributes;
};

inline toki::Array<VertexBindingDescription, 1> Vertex::vertex_bindings = {
	{ 0, sizeof(Vertex), VertexInputRate::VERTEX }
};

inline toki::Array<VertexAttributeDescription, 3> Vertex::vertex_attributes = {
	VertexAttributeDescription{ 0, 0, VertexFormat::FLOAT3, offsetof(Vertex, position) },
	VertexAttributeDescription{ 1, 0, VertexFormat::FLOAT3, offsetof(Vertex, normals) },
	VertexAttributeDescription{ 2, 0, VertexFormat::FLOAT2, offsetof(Vertex, uv) },
};

struct FontVertex {
	Vector3 position;
	Vector2 uv;

	static toki::Array<VertexBindingDescription, 1> vertex_bindings;
	static toki::Array<VertexAttributeDescription, 2> vertex_attributes;
};

inline toki::Array<VertexBindingDescription, 1> FontVertex::vertex_bindings = {
	{ 0, sizeof(FontVertex), VertexInputRate::VERTEX }
};

inline toki::Array<VertexAttributeDescription, 2> FontVertex::vertex_attributes = {
	VertexAttributeDescription{ 0, 0, VertexFormat::FLOAT3, offsetof(FontVertex, position) },
	VertexAttributeDescription{ 1, 0, VertexFormat::FLOAT2, offsetof(FontVertex, uv) },
};

}  // namespace toki
