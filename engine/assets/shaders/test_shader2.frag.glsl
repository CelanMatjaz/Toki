#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform texture2D inTexture;
layout(set = 0, binding = 1) uniform sampler inSampler;

void main() {
   outColor = texture(sampler2D(inTexture, inSampler), inUV);
}
