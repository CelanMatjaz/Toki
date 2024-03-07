#version 460
 
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    gl_Position = vec4(inPosition, 1.0) * PushConstants.mvp;
    outColor = inColor;
}
