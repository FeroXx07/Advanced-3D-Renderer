#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 3) in vec3 sViewDir; // In worldspace

layout (binding = 0) uniform sampler2D uRTColor; 
layout (binding = 1) uniform sampler2D uRTPosition; 
layout (binding = 2) uniform sampler2D uRTNormals; 
layout (binding = 3) uniform sampler2D uRTSpecularRoughness; 

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
//layout(location = 1) out vec4 rt1; // Position
//layout(location = 2) out vec4 rt2; // Normals
//layout(location = 3) out vec4 rt3; // Emissive + lightmaps
//layout(location = 4) out vec4 rt4; // Specular, roughness
//layout(location = 5) out vec4 rt3; // Final result

void main()
{
    vec3 fragPos = texture(uRTPosition, sTextCoord).rgb;
    vec3 normal = texture(uRTNormals, sTextCoord).rgb;
    vec3 albedo = texture(uRTColor, sTextCoord).rgb;
	float specularStrength = texture(uRTSpecularRoughness, sTextCoord).r;
	
    float ambientStrength = 0.1;
    
    vec3 result = albedo * ambientStrength; 
    vec3 nViewDir = normalize(uCameraPosition - fragPos);
	
	for (int i = 0; i < uLightCount; ++i)
	{
		float attenuation = 0.0f;
		vec3 lightDir = vec3(0.0f, 0.0f, 0.0f);

		if (uLight[i].type == 1)
		{
			float distance = length(uLight[i].position - fragPos);
			if(distance > uLight[i].radius)
			{
				continue;
			}

			attenuation = 1.0 / (1.0 + uLight[i].linear * distance + uLight[i].quadratic * distance * distance);

			lightDir = normalize(uLight[i].position - fragPos);
		}
		else
		{
			lightDir = normalize(uLight[i].direction);
		}
		 // diffuse
		float diff = max(dot(normal, lightDir), 0.0); // Clamp to 0.0
		vec3 diffuse = diff * albedo * uLight[i].color;
		
		// specular
		vec3 halfwayDir = normalize(lightDir + nViewDir);  
		float shininess = 32;
		float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
		vec3 specular = spec * specularStrength * uLight[i].color;
		
		if (uLight[i].type == 1)
		{
			// attenuation
			diffuse *= attenuation;
			specular *= attenuation;
			
			result += (diffuse + specular/*+ specular*/);
		}
		else
		{
			result += (diffuse + specular);
		}
		
		
	}
	
	rt0 = vec4(result, 1.0);
}