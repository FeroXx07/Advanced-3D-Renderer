#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 5) in vec2 sViewDir; 

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout (binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;     		
	uint uLightCount; 	
	struct Light					
	{
		uint type;			
		vec3 color;					
		vec3 direction;				
		vec3 position;				
	} uLight[16];     		   		
}; 

layout(location = 0) out vec4 oColor;

void main()
{
	// TODO: Sum all light contributions up to set oColor final value
    oColor = texture(uTexture, sTextCoord);
}