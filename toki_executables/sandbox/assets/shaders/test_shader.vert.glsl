#version 450

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out int out_index;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

void main() {
    gl_Position = vec4(in_position, 1.0);
    gl_Position.y *= -1;
    out_color = vec3(0.4, 0.8, 0.2);
    out_uv = in_uv;
    out_index = gl_VertexIndex;
}
