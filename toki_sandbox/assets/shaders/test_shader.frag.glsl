#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;

void main() {
    outColor = outColor1 = vec4(fragColor, 1.0);
}
