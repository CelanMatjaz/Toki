#version 450

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outUv;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec3 instancePosition;
layout (location = 3) in vec3 instanceScale;
layout (location = 4) in vec3 instanceRotation;
layout (location = 5) in vec4 instanceColor;

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    mat3 rotX, rotY, rotZ;

    // X axis rotation
    float a1 = cos(instanceRotation.x);
    float a2 = sin(instanceRotation.x);

    rotX[0] = vec3(1.0, 0.0, 0.0);
    rotX[1] = vec3(0.0, a1, a2);
    rotX[2] = vec3(0.0, -a2, a1);

    // Y axis rotation
    a1 = cos(instanceRotation.y);
    a2 = sin(instanceRotation.y);

    rotY[0] = vec3(a1, 0.0, a2);
    rotY[1] = vec3(0.0, 1.0, 0.0);
    rotY[2] = vec3(-a2, 0.0, a1);

    // Z axis rotation
    a1 = cos(instanceRotation.z);
    a2 = sin(instanceRotation.z);    

    rotZ[0] = vec3(a1, a2, 0.0);
    rotZ[1] = vec3(-a2, a1, 0.0);
    rotZ[2] = vec3(0.0, 0.0, 1.0);

    mat3 rotationMat = rotZ * rotY * rotX;

    vec3 localPosition = vec3(position * instanceScale * rotationMat);
    vec4 instancePos = vec4(instancePosition + (localPosition ), 1.0);

    gl_Position = PushConstants.mvp * instancePos;
    outColor = instanceColor;
    outUv = outUv;
}