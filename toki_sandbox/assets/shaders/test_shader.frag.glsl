#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    outColor = outColor1 = vec4(fragColor, 1.0);
}
