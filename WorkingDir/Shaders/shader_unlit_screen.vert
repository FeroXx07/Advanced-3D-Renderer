#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; 
layout(location = 2) in vec2 aTextCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;


// Can use the same locations for out and in because the belong the different stages in the pipeline.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal; // In worldspace
layout(location = 2) out vec2 vTextCoord; // In worldspace
layout(location = 5) out vec3 vViewDir; // In worldspace

void main() {
	vTextCoord = aTextCoord;
    gl_Position = vec4(aPosition, 1.0);
}
