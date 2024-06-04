#version 460
 
layout(location = 0) out vec2 outUV;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(set = 1, binding = 0) uniform RotationZ {
    float rotation;
} rotationZ;

void main() {
    float a = cos(rotationZ.rotation);
    float b = sin(rotationZ.rotation);

    mat3 rotZ;
    rotZ[0] = vec3(a, b, 0.0);
    rotZ[1] = vec3(-b, a, 0.0);
    rotZ[2] = vec3(0.0, 0.0, 1.0);

    gl_Position = vec4(inPosition * rotZ, 1.0);
    outUV = inUV;
}
