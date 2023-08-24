#version 450

layout (set = 0, binding = 0) uniform UniformData {
    mat4 projection;
    mat4 view;
    mat4 model;
} ubo;

void main() {
    gl_Position = vec4(0, 0, 0, 0);
}