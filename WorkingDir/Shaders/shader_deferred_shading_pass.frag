#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 3) in vec3 sViewDir; // In worldspace

layout (binding = 0) uniform sampler2D uRTColor; 
layout (binding = 1) uniform sampler2D uRTPosition; 
layout (binding = 2) uniform sampler2D uRTNormals; 
layout (binding = 3) uniform sampler2D uRTSpecularRoughness; 
layout (binding = 4) uniform sampler2D uRTSSAO; 
layout (binding = 5) uniform sampler2D uRTBump; 
layout (binding = 6) uniform sampler2D uRTTangent; 

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

// location in this context are the indices of the draw buffers array.
layout(location = 0) out vec4 rt0; // Color -> drawBuffers[0] = GL_COLOR_ATTACHMENT#; where # = n  refers to a texture in a frame buffer
//layout(location = 1) out vec4 rt1; // Position
//layout(location = 2) out vec4 rt2; // Normals
//layout(location = 3) out vec4 rt3; // Emissive + lightmaps
//layout(location = 4) out vec4 rt4; // Specular, roughness
//layout(location = 5) out vec4 rt3; // Final result

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, out bool hasBump)
{ 
	if (texture(uRTBump, texCoords).r == 0)
	{
		hasBump = false;
		return texCoords;
	}

    float height = 1 - texture(uRTBump, texCoords).r;
	hasBump = true;
    return texCoords - viewDir.xy * (height * material.heightScale);        
}

vec2 ParallaxMapping2(vec2 texCoords, vec3 viewDir, out bool hasBump)
{ 
	if (texture(uRTBump, texCoords).r == 0)
	{
		hasBump = false;
		return texCoords;
	}

	hasBump=true;
	// number of depth layers
    const float minLayers = 128;
    const float maxLayers = 254;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * material.heightScale;  
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = 1-texture(uRTBump, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = 1-texture(uRTBump, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = 1-texture(uRTBump, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
	vec3 normal = texture(uRTNormals, sTextCoord).rgb;
	vec3 tangent = texture(uRTTangent, sTextCoord).rgb;
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = transpose(mat3(bitangent, tangent, normal));

    vec3 fragPos = texture(uRTPosition, sTextCoord).rgb;
	vec3 tangentFragPos = TBN * fragPos;
	vec3 tangentViewPos = TBN * uCameraPosition;
	vec3 tangentViewDir = normalize(tangentViewPos - tangentFragPos);

	vec3 nViewDir = normalize(uCameraPosition - fragPos);

	bool hasBump = false;
    vec2 texCoords = ParallaxMapping2(sTextCoord, tangentViewDir, hasBump);
	if (hasBump == true)
	{
		if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
			discard;
	}

	normal = texture(uRTNormals, texCoords).rgb;
	fragPos = texture(uRTPosition, texCoords).rgb;
    vec3 albedo = texture(uRTColor, texCoords).rgb;
	float specularStrength = texture(uRTSpecularRoughness, texCoords).r;
	float ambientOcclusion = texture(uRTSSAO, texCoords).r;


    float ambientStrength = 0.1;
    
    vec3 result = albedo * ambientStrength * ambientOcclusion; 
	
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