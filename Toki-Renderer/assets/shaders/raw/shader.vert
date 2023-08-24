#version 450

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outUv;
layout (location = 2) out uint outTexIndex;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 positionInstance;
layout (location = 4) in float scaleInstance;
layout (location = 5) in uint texIndex;

layout (set = 0, binding = 0) uniform UniformData {
    mat4 projection;
    mat4 view;
    mat4 model;
} ubo;

layout(set = 2, binding = 0) uniform sampler2D heightMap;

void main() {
    gl_Position = (ubo.projection * ubo.view * ubo.model * (vec4(positionInstance + (position * scaleInstance), 1.0)));
    outUv = uv;
    outTexIndex = texIndex;
    outColor = color;
    outColor = vec4(position.y / 32, position.y / 32, position.y / 32, 1.0);
}