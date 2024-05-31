#version 460
 
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    outColor = vec4(1.0);
}
