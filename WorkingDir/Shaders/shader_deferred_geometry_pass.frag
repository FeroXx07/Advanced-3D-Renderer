#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
//layout(location = 3) in vec3 sTangent;
//layout(location = 4) in vec3 sBitangent;
layout(location = 5) in vec3 sViewDir; // In worldspace

layout (binding = 0) uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

struct Light					
{
	uint type;			
	vec3 color;					
	vec3 direction;				
	vec3 position;			
	float constant;
    float linear;
    float quadratic;
	float radius;	
};

struct Material
{
	vec3 albedo;
	vec3 emissive;
	float smoothness;
	bool hasAlbedoTexture;
	bool hasEmissiveTexture;
	bool hasSpecularTexture;
	bool hasNormalsTexture;
	bool hasBumpTexture;
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

layout(binding = 2, std140) uniform MaterialParams
{
	Material material;
};

// location in this context are the indices of the draw buffers array.
layout(location = 0) out vec4 rt0; // Color -> drawBuffers[0] = GL_COLOR_ATTACHMENT#; where # = n  refers to a texture in a frame buffer
layout(location = 1) out vec4 rt1; // Position
layout(location = 2) out vec4 rt2; // Normals
layout(location = 3) out vec4 rt3; // Emissive + lightmaps
layout(location = 4) out vec4 rt4; // Specular, roughness

void main()
{
    rt0 = texture(uTexture, sTextCoord);
	rt1 = vec4(sPosition, 1.0);
	rt2 = vec4(sNormal, 1.0);
}