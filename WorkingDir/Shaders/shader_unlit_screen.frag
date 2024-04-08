#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 5) in vec2 sViewDir; 

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, sTextCoord);;
}