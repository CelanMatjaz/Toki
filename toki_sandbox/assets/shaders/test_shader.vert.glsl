#version 450

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out int out_index;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_position_instance;
layout(location = 3) in vec3 in_position_color;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

layout(set = 0, binding = 0) uniform color {
    vec3 color;
} test_color;

void main() {
    gl_Position = PushConstants.mvp * vec4(in_position + in_position_instance, 1.0);
    out_color = test_color.color * in_position_color;
    out_uv = in_uv;
    out_index = gl_VertexIndex;
}
