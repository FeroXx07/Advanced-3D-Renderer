#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 3) in vec3 sViewDir; // In worldspace
layout(location = 4) in mat3 sTBN; 

layout (binding = 1) uniform sampler2D uRTPosition; 
layout (binding = 2) uniform sampler2D uRTNormals; 
layout (binding = 3) uniform sampler2D uSSAONoise; 

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

// location in this context are the indices of the draw buffers array.
layout(location = 0) out vec4 rt0; // Color -> drawBuffers[0] = GL_COLOR_ATTACHMENT#; where # = n  refers to a texture in a frame buffer

void main()
{
	// Setting inputs for SSAO
	vec3 fragPos = texture(uRTPosition, sTextCoord).rgb;
    vec3 normal = texture(uRTNormals, sTextCoord).rgb;
	vec3 randomVec = normalize(texture(uSSAONoise, sTextCoord * ssao.noiseScale).xyz);
	
	// create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
	
	/*// The kernels iteration task
	// iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < ssao.kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * ssao.samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * ssao.radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = ssao.projectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(uRTPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, ssao.radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + ssao.bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / ssao.kernelSize);
	
	rt0 = vec4(occlusion);*/
	
	rt0 = vec4(0.5, uLightCount, 0.0, 1.0);
}