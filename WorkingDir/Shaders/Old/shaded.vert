#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(location = 5) out vec2 vTextCoord;

void main() {
    vTextCoord = aTextCoord;

    float clippingScale = 5.0;
    gl_Position = vec4(aPosition, clippingScale);

    // Patrick looks away from the camera by default
    gl_Position.z = -gl_Position.z;
}
