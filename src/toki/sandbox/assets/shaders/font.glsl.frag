#version 460

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

void main() {
    float alpha = texture(tex_sampler, in_uv).r;
    out_color = vec4(vec3(1.0, 1.0, 1.0) * (1-alpha), alpha);
}
