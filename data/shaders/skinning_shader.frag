#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vColor;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec3 ambient;
uniform vec3 diffuse;

#include "dirLight.frag"

uniform DirectionalLight sunLight;

void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = texture2D(gSampler, vTexCoord);

	//vec4 vMixedColor = vTexColor * vColor;
	vec4 vMixedColor = vTexColor;
    vec4 vDirLightColor = GetDirectionalLightColor(sunLight, vNormalized);

	outputColor = vMixedColor;
}