#version 460

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_position;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

vec3 light_pos = vec3(1.0, 2.0, 3.0);
vec3 light_color = vec3(0.4, 0.8, 0.2);
vec3 object_color = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 light_dir = normalize(light_pos - in_position);

    float diff = max(dot(normalize(in_normals), light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    vec3 color = diffuse * vec3(texture(tex_sampler, in_uv));

    out_color = vec4(color, 1.0);
}
