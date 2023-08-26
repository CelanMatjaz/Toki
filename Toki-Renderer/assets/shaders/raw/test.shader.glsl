#type vert
#version 450

layout (location = 0) out vec2 outUv;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout (location = 3) in vec3 instancePosition;

layout(push_constant) uniform constants {
    mat4 mvp;
	vec2 offsetPosition;
} PushConstants;

void main() {
    outUv = uv;
    gl_Position = PushConstants.mvp * vec4(position.x + instancePosition.x + PushConstants.offsetPosition.x, position.y + instancePosition.y + PushConstants.offsetPosition.y, 0.0, 1.0);
}

// ---------------------------

#type frag
#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 uv;

layout(set = 1, binding = 0) uniform sampler2D texSampler[32];

layout(set = 0, binding = 0) uniform color {
    vec4 color;
    uint textureIndex;
} addedColor;

void main() {
	outFragColor = vec4(1, 1, 1, 1) * addedColor.color;	
    outFragColor = texture(texSampler[addedColor.textureIndex], vec2(uv.x, -uv.y));
}