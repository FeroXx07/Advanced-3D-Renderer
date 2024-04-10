#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; // In localspace
layout(location = 2) in vec2 aTextCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

struct Light					
{
	uint type;			
	vec3 color;					
	vec3 direction;				
	vec3 position;				
};

layout (binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;     		
	uint uLightCount; 	
	Light uLight[16];     		   		
};

layout(binding = 1, std140) uniform LocalParams
{
	vec4 uColor;
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
	mat3 uNormalMatrix;
};

// Can use the same locations for out and in because the belong the different stages in the pipeline.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal; // In worldspace
layout(location = 2) out vec2 vTextCoord; // In worldspace
layout(location = 5) out vec3 vViewDir; // In worldspace

void main() {
    vTextCoord = aTextCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal = normalize(vec3(uWorldMatrix * vec4(aNormal, 0.0)));
	vViewDir = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}
