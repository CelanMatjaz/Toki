#version 450

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_position_instance;
layout(location = 2) in vec3 in_position_color;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

layout(set = 0, binding = 0) uniform color {
    vec3 color;
} test_color;

void main() {
    gl_Position = PushConstants.mvp * vec4(in_position + in_position_instance, 1.0);
    fragColor = test_color.color * in_position_color + float(gl_VertexIndex) * 0.01;
}
