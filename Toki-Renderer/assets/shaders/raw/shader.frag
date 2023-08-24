#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 uv;
layout (location = 2) in flat uint textureIndex;

layout(set = 1, binding = 0) uniform sampler2D texSampler[2];

void main() {
	outFragColor = texture(texSampler[textureIndex], vec2(uv.x, uv.y * -1));
	outFragColor = inColor;
}