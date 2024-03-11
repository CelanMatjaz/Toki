#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(set = 1, binding = 0) uniform texture2D textureIn;
layout(set = 1, binding = 1) uniform sampler samplerIn;

void main() {
   outColor = inColor;
   outColor = texture(sampler2D(textureIn, samplerIn), vec2(inUV.x, -inUV.y));
}
