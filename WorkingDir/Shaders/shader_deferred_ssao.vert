#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; 
layout(location = 2) in vec2 aTextCoord;

layout(location = 2) out vec2 vTextCoord;

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

struct ScreenSpaceAmbientOcclusion
{
	vec3 samples[64];
	uint kernelSize;
	float radius;
	float bias;
	vec2 noiseScale;
	mat4 projectionMatrix;
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

layout(binding = 3, std140) uniform SSAOParams
{
	ScreenSpaceAmbientOcclusion ssao;
};

void main() {
	vTextCoord = aTextCoord;
    gl_Position = vec4(aPosition, 1.0);
}
