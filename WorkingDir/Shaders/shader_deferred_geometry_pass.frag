#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 3) in vec3 sViewDir; // In worldspace
layout(location = 4) in mat3 sTBN; 

layout (binding = 0) uniform sampler2D uTextureDiffuse; // www.khronos.org/opengl/wiki/Uniform_(GLSL)
layout (binding = 1) uniform sampler2D uTextureBump; 
layout (binding = 2) uniform sampler2D uTextureSpecular; 

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
	mat4 uViewMatrix;
	mat4 uProjectionMatrix;	
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
layout(location = 1) out vec4 rt1; // Position world space
layout(location = 2) out vec4 rt2; // Normals
layout(location = 3) out vec4 rt3; // Specular, roughness
layout(location = 4) out vec4 rt4; // Position view space

void main()
{
	vec4 objectColor = vec4(material.albedo, 1.0);
	if (material.hasAlbedoTexture)
	{
		objectColor = texture(uTextureDiffuse, sTextCoord);
	}
	
	vec3 normal = normalize(sNormal);
	if (material.hasNormalsTexture)
	{
		normal = texture(uTextureBump, sTextCoord).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(sTBN * normal);
	}
	
	float specularStrength = 0.8;
	if (material.hasSpecularTexture)
	{
		specularStrength = texture(uTextureSpecular, sTextCoord).r;
	}
	
    rt0 = objectColor;
	rt1 = vec4(sPosition, 1.0);
	rt2 = vec4(normal, 1.0);
	rt3 = vec4(specularStrength, specularStrength, specularStrength, 1.0);
}