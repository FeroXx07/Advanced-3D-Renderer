#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; 
layout(location = 2) in vec2 aTextCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

// Can use the same locations for out and in because the belong the different stages in the pipeline.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal; // In worldspace
layout(location = 2) out vec2 vTextCoord; // In worldspace

void main() {
    vTextCoord = aTextCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	 // Homogenous coordinate 0.0 because we don't want to translate the normal vector.
	vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0));
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}
