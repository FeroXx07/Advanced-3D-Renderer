#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 5) in vec2 sViewDir; 

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

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

layout(location = 0) out vec4 oColor;

void main()
{
	// TODO: Sum all light contributions up to set oColor final value
    oColor = uColor;
}