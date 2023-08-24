#version 450

layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec4 inColor;

float convert(float val) {
	return cos(val) * 3 - 2;
}

void main() {
	outFragColor = vec4(0,0,0, 1.0);
}