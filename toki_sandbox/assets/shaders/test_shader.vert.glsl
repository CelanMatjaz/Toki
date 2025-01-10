#version 450

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec3 in_position;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    gl_Position = PushConstants.mvp * vec4(in_position, 1.0);
    fragColor = vec3(1.0, 1.0, 1.0);
}
