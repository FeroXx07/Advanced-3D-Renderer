#version 430

layout(location = 0) in vec3 aPosition; 
layout(location = 1) in vec2 aTextCoord;
layout(location = 3) out vec2 vTextCoord;

void main()
{
    vTextCoord = aTextCoord;
    gl_Position = vec4(aPosition, 1.0);
}