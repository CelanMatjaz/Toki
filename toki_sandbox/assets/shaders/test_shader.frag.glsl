#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in flat int in_index;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_color2;

layout(push_constant) uniform constants {
    mat4 mvp;
} PushConstants;

layout(set = 0, binding = 1) uniform sampler2D tex;

void main() {
    out_color2 = vec4(in_color, 1.0);
    out_color2 = texture(tex, in_uv);
    out_color = texture(tex, in_uv) + (in_index) * 0.01;
    out_color.a = 1.0;
}
