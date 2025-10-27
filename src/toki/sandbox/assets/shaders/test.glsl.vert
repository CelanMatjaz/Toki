#version 460

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normals;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec3 out_position;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_uv;

layout(set = 0, binding = 0) uniform MyUniform {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 color;
} ubo;

void main() {
    out_color = ubo.color;
    out_normals = normalize(mat3(ubo.model) * in_normals);
    out_uv = in_uv;
    vec4 world_pos = ubo.model * vec4(in_position, 1.0);
    out_position = world_pos.xyz;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
}
