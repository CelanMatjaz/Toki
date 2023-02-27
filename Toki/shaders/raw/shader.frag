#version 450

layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec4 inColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	outFragColor = inColor;
	outFragColor.w = 1.0;
	outFragColor = texture(texSampler, fragTexCoord) ;
}