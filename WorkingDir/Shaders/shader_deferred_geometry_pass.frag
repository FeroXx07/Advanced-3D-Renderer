#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
//layout(location = 3) in vec3 sTangent;
//layout(location = 4) in vec3 sBitangent;
layout(location = 5) in vec2 sViewDir; // In worldspace

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

// location in this context are the indices of the draw buffers array.
layout(location = 0) out vec4 rt0; // Color -> drawBuffers[0] = GL_COLOR_ATTACHMENT#; where # = n  refers to a texture in a frame buffer
layout(location = 1) out vec4 rt1; // Position
layout(location = 2) out vec4 rt2; // Normals
layout(location = 3) out vec4 rt3; // Emissive + lightmaps
layout(location = 4) out vec4 rt4; // Specular, roughness

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    rt0 = texture(uTexture, sTextCoord);
	rt1 = vec4(sPosition, 1.0);
	rt2 = vec4(sNormal, 1.0);
	rt3 = texture(uTexture, sTextCoord);
}