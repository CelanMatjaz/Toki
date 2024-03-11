#version 460
 
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUv;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

layout(set = 0, binding = 0) uniform UniformBuffer {
    mat4 mvp;
} uniformBuffer;

layout(set = 0, binding = 1) uniform Offset {
    vec3 offset;
} offset;

void main() {
    gl_Position = uniformBuffer.mvp * vec4(inPosition + offset.offset, 1.0);
    outColor = inColor;
    outUv = inUV;
}
