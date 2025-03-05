#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in flat int in_index;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(in_color, 1.0);
}
