#version 430


uniform vec2 viewportSize;
unifrom mat4 modelViewMatrix;
uniform mat4 viewMatrixInv;
uniform mat4 projectionMatrixInv;
uniform sampler2D reflectionMap;
uniform sampler2D refractionMap;
uniform sampler2D reflectionDepth;
uniform sampler2D refractionDepth;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;

in Data {
	vec3 positionViewspace;
	vec3 normalViewspace;
} FSIn;

out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 + (1.0 - F0) *  pow(1.0 - cosTheta, 5.0);
}

vec3 reconstructPixelPosition(float depth){
	vec2 texCoords = gl_FragCoord.xy / viewportSize;
	vec3 positionNDC = vec3(texCoords * 2.0 - vec2(1.0), depth * 2.0 - 1.0);
	vec4 positionEyescape = projectionMatrixInv * vec4(positionNDC, 1.0);
	positionEyescape.xyz /= positionEyescape.w;
	return positionEyescape.xyz;
}

void main(){
	vec3 N = normalize(FSIn.normalViewspace);
	vec3 V = normalize(-FSIn.positionViewspace);
	vec3 Pw = vec3(viewMatrixInv *  vec4(FSIn.positionViewspace, 1.0));
	vec2 texCoord = gl_FragCoord.xy / viewportSize;

	const vec2 waveLength = vec2(2.0);
	const vec2 waveStrength = vec2(0.05);
	const float turbidityDistance = 10.0;

	vec2 distortion = (2.0 * texture(dudvMap,Pw.xz / waveLength).rg - vec2(1.0)) * waveStrength + waveStrength/7;

	vec2 reflectionTexCoord = vec2(texCoord.s, 1.0 - texCoord.t) + distortion;
	vec2 refractionTexCoord = texCoord + distortion;
	vec3 reflectionColor = texture(reflectionMap, reflectionTexCoord).rgb;
	vec3 refractionColor = texture(refractionMap, refractionTexCoord).rgb;

	float distortedGroundDepth = texture(refractionDepth, refractionTexCoord).x;
	vec3 distortedGroundPosViewspace = reconstructPixelPosition(distortedGroundDepth);
	float distortedWaterDepth = FSIn.positionViewspace.z - distortedGroundPosViewspace.z;
	float tintFactor = clamp(distortedWaterDepth / turbidityDistance, 0.0 ,1.0);
	vec3 waterColor = vec3(0.25,0.4,0.6);
	refractionColor = mix(refractionColor,waterColor,tintFactor);

	vec3 F0 = vec3(0.1);
	vec3 F = fresnelShlick(max(0.0,dot(V,N)), F0);
	outColor.rgb = mix(refractionColor,reflectionColor,F);
	outColor.a = 1.0;
}
