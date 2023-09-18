#type vert
#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout (location = 3) in vec3 instancePosition;
layout (location = 4) in vec3 instanceRotation;
layout (location = 5) in vec3 instanceScale;
layout (location = 6) in vec4 instanceColor;
layout (location = 7) in float id;

layout(push_constant) uniform constants {
	mat4 mvp;
    vec4 color;
} PushConstants;

void main() {
    gl_Position = PushConstants.mvp * vec4(position + instancePosition, 1.0);
    outColor = PushConstants.color;
}

#type frag
#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec4 inColor;

void main() {
    outFragColor = inColor;
}