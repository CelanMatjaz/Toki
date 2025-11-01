#version 460

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

layout(set = 0, binding = 0) uniform MyUniform {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 color;
} ubo;

void main() {
    out_color = ubo.color;
    out_uv = in_uv;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
    gl_Position.z = 0.0;
}
