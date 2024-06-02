#version 430

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
layout(location = 1) in vec3 aNormal; // In local tangent space
layout(location = 2) in vec2 aTextCoord;
layout(location = 3) in vec3 aTangent; // In local tangent space
layout(location = 4) in vec3 aBitangent; // In local tangent space
layout(location = 4) out vec3 vTangent; 
layout(location = 5) out mat3 vTBN;

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
	float heightScale;
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

// Can use the same locations for out and in because the belong the different stages in the pipeline.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal; // In world tangent space
layout(location = 2) out vec2 vTextCoord; // In worldspace
layout(location = 3) out vec3 vViewDir; // In worldspace
layout(location = 4) out mat3 vTBN; 

void main() {
    vTextCoord = aTextCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	mat3 normalMatrix = transpose(inverse(mat3(uViewMatrix * uWorldMatrix)));

	vec3 T = normalize(vec3(uWorldMatrix * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(uWorldMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(uWorldMatrix * vec4(aNormal,    0.0)));

	/*vec3 T = normalize(vec3(uWorldMatrix * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(uWorldMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(uWorldMatrix * vec4(aNormal,    0.0)));*/
    vTBN = mat3(T, B, N);
	
	vNormal = N;
	vTangent = T;

	vViewDir = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
	float a = material.albedo.x;
}
