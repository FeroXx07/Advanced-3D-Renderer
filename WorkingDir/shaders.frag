#version 430

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout(location = 0) out vec4 oColor;
layout(location = 3) in vec2 sTextCoord;

void main()
{
    oColor = texture(uTexture, sTextCoord);
}