#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 inColor;

layout(set = 1, binding = 0) uniform texture2D textureIn;
layout(set = 1, binding = 1) uniform sampler samplerIn;

void main() {
   outColor = inColor;
}
