#version 430

layout(location = 0) in vec3 sPosition; // In worldspace
layout(location = 1) in vec3 sNormal; // In worldspace
layout(location = 2) in vec2 sTextCoord; 
layout(location = 5) in vec3 sViewDir; 

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout (binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;     		
	uint uLightCount; 	
	struct Light					
	{
		uint type;			
		vec3 color;					
		vec3 direction;				
		vec3 position;				
	} uLight[16];     		   		
}; 

layout(binding = 1, std140) uniform LocalParams
{
	vec4 uColor;
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

layout(location = 0) out vec4 oColor;

void main()
{
	// TODO: Sum all light contributions up to set oColor final value
	Light directionalLight;
	
	vec3 result;
	vec3 objectColor = texture(uTexture, sTextCoord).xyz;
	// Get the first directional light. Not accounting for multiple directional lights for now.
	for (int i = 0; i < uLightCount; ++i)
	{
		// To calculate PHONG = Ambient + Diffuse + Specular
		// Calculate Ambient
		float ambientStrength = 0.1;
		vec3 ambient = ambientStrength * uLight[i].color;
		
		// Calculate Diffuse lightning
		vec3 lightDir = normalize(uLight[i].position - sPosition);  
		float diff = max(dot(sNormal, lightDir), 0.0); // Clamp to 0.0
		vec3 diffuse = diff * uLight[i].color;
		
		// Calculate Specular lightning
		float specularStrength = 0.8;
		vec3 nViewDir = normalize(sViewDir);
		// Negate the lightDir vector. The reflect function expects the first vector to point from the light source towards the fragment's position
		vec3 reflectDir = reflect(-lightDir, sNormal);  
		// 32 value is the shininess value of the highlight. The higher the shininess value of an object, 
		// the more it properly reflects the light instead of scattering it all around and thus the smaller the highlight becomes. 
		float shininess = 32;
		float spec = pow(max(dot(nViewDir, reflectDir), 0.0), shininess);
		vec3 specular = specularStrength * spec * uLight[i].color; 
		
		result += (ambient + diffuse + specular) * vec3(objectColor.x, objectColor.y, objectColor.z);
	}
	
    oColor = vec4(result, 1);
}