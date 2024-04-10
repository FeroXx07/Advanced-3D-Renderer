#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
//layout(location = 3) in vec3 sTangent;
//layout(location = 4) in vec3 sBitangent;
layout(location = 5) in vec2 sViewDir; 

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout(location = 0) out vec4 rt0; // Color
layout(location = 1) out vec4 rt1; // Specular, roughness
layout(location = 2) out vec4 rt2; // Normals
layout(location = 3) out vec4 rt3; // Emissive + lightmaps
layout(location = 4) out vec4 rt4; // Position

void main()
{
    rt0 = texture(uTexture, sTextCoord);
}