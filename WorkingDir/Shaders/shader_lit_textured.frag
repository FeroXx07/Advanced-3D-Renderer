#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 3) in vec3 sViewDir; 
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

layout(location = 0) out vec4 oColor;

void main()
{
	vec3 result;
	
	vec3 objectColor = material.albedo;
	if (material.hasAlbedoTexture)
	{
		objectColor = texture(uTextureDiffuse, sTextCoord).xyz;
	}
	
	vec3 normal = sNormal;
	if (material.hasBumpTexture)
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
	
	// Get the first directional light. Not accounting for multiple directional lights for now.
	for (int i = 0; i < uLightCount; ++i)
	{
		float attenuation = 0.0f;
		vec3 lightDir = vec3(0.0f, 0.0f, 0.0f);

		if (uLight[i].type == 1)
		{
			float distance = length(uLight[i].position - sPosition);
			if(distance > uLight[i].radius)
			{
				continue;
			}

			attenuation = 1.0 / (1.0 + uLight[i].linear * distance + uLight[i].quadratic * distance * distance);

			lightDir = normalize(uLight[i].position - sPosition);
		}
		else
		{
			lightDir = normalize(uLight[i].direction);
		}
		
		// To calculate PHONG = Ambient + Diffuse + Specular
		// Calculate Ambient
		float ambientStrength = 0.01;
		vec3 ambient = ambientStrength * objectColor;
		
		// Calculate Diffuse lightning
	
		float diff = max(dot(normal, lightDir), 0.0); // Clamp to 0.0
		vec3 diffuse = diff * objectColor* uLight[i].color;
		
		// Calculate Specular lightning
		
		vec3 nViewDir = normalize(sViewDir);
		// Negate the lightDir vector. The reflect function expects the first vector to point from the light source towards the fragment's position
		vec3 reflectDir = reflect(-lightDir, normal);  
		// 32 value is the shininess value of the highlight. The higher the shininess value of an object, 
		// the more it properly reflects the light instead of scattering it all around and thus the smaller the highlight becomes. 
		float shininess = 32;
		float spec = pow(max(dot(nViewDir, reflectDir), 0.0), shininess);
		vec3 specular = specularStrength * spec * uLight[i].color; 
		
		if (uLight[i].type == 1)
		{
			// attenuation
			diffuse *= attenuation;
			specular *= attenuation;
			
			result += (ambient + diffuse + specular);
		}
		else
		{
			result += (ambient + diffuse + specular);
		}
		
		
	}
	
    oColor = vec4(result, 1);
}

//void main()
// {
	// // TODO: Sum all light contributions up to set oColor final value
	// Light directionalLight;
	
	// vec3 result;
	// vec3 objectColor = texture(uTexture, sTextCoord).xyz;
	// // Get the first directional light. Not accounting for multiple directional lights for now.
	// for (int i = 0; i < uLightCount; ++i)
	// {
		// // To calculate PHONG = Ambient + Diffuse + Specular
		// // Calculate Ambient
		// float ambientStrength = 0.1;
		// vec3 ambient = ambientStrength * uLight[i].color;
		
		// // Calculate Diffuse lightning
		// vec3 lightDir = normalize(uLight[i].position - sPosition);  
		// float diff = max(dot(sNormal, lightDir), 0.0); // Clamp to 0.0
		// vec3 diffuse = diff * uLight[i].color;
		
		// // Calculate Specular lightning
		// float specularStrength = 0.8;
		// vec3 nViewDir = normalize(sViewDir);
		// // Negate the lightDir vector. The reflect function expects the first vector to point from the light source towards the fragment's position
		// vec3 reflectDir = reflect(-lightDir, sNormal);  
		// // 32 value is the shininess value of the highlight. The higher the shininess value of an object, 
		// // the more it properly reflects the light instead of scattering it all around and thus the smaller the highlight becomes. 
		// float shininess = 32;
		// float spec = pow(max(dot(nViewDir, reflectDir), 0.0), shininess);
		// vec3 specular = specularStrength * spec * uLight[i].color; 
		
		// result += (ambient + diffuse + specular) * vec3(objectColor.x, objectColor.y, objectColor.z);
	// }
	
    // oColor = vec4(result, 1);
// }