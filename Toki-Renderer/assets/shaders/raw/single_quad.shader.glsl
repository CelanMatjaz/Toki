#type vert
#version 450

layout (location = 0) out vec2 outUV;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    gl_Position = PushConstants.mvp * vec4(position, 1.0);
    outUV = uv;
}

#type frag
#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 uv;

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 0, binding = 1) uniform Lighting {
    float brightness;
} lighting;

void main() {
    outFragColor = texture(texSampler, vec2(uv.x, uv.y)) * lighting.brightness;
}