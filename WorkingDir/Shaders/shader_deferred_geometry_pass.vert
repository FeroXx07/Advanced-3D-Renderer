#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; // In local tangent space
layout(location = 2) in vec2 aTextCoord;
layout(location = 3) in vec3 aTangent; // In local tangent space
layout(location = 4) in vec3 aBitangent; // In local tangent space

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

// Can use the same locations for out and in because the belong the different stages in the pipeline.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal; // In world tangent space
layout(location = 2) out vec2 vTextCoord; // In worldspace
layout(location = 3) out vec3 vTangent; // In world tangent space
layout(location = 4) out vec3 vBitangent; // In world tangent space
layout(location = 5) out vec3 vViewDir; // In worldspace

void main() {
    vTextCoord = aTextCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal = normalize(vec3(uWorldMatrix * vec4(aNormal, 0.0)));
	vViewDir = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
	float a = material.albedo.x;
}
