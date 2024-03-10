#version 460
 
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

layout(set = 0, binding = 0) uniform UniformBuffer {
    mat4 mvp;
} uniformBuffer;

void main() {
    gl_Position = uniformBuffer.mvp * vec4(inPosition, 1.0);
    outColor = inColor;
}
