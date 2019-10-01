#version 430

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
in vec3 WorldNormal;

out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

#include "Lights.frag"

const int maxDirectionalLights = 50;
const int maxSpotLights = 50;

layout(std140,binding = 1) uniform LightSourceBlock
{
    DirectionalLight directionalLights[maxDirectionalLights];
	Spotlight spotLights[maxSpotLights];

	int directionalLightNum;
	int spotLightNum;
};

uniform samplerCube envirMap;//Cube Map #akira
uniform vec3 CameraPos;//Camera position #akira

void main()
{
	
	//###Region akira
	vec3 ViewVector=normalize (vWorldPos-CameraPos);
	vec3 unitNormal=normalize(vNormal);
	
	vec4 vTexColor = texture2D(gSampler, vTexCoord);
	vec3 vTexColorRgb = vTexColor.rgb;
	// CaculateLights
	vec3 lightColor = vec3(0.0,0.0,0.0);
	
	
	for(int i = 0 ; i < directionalLightNum ; i++){
		if(directionalLights[i].enable){
			lightColor +=CaculDirectionalLightColor(vWorldPos,unitNormal,CameraPos,directionalLights[i],vTexColorRgb);
		}
	}
	
	for(int i = 0 ; i < spotLightNum ; i++){
		if(spotLights[i].enable){
			//lightColor+= CaculSpotLightColor(vWorldPos,unitNormal,CameraPos,spotLights[i],vTexColorRgb);
			lightColor +=CaculSmoothSpotLight(vWorldPos,unitNormal,CameraPos,spotLights[i],vTexColorRgb);
		}
	}

	vec4 totalColor= vec4(lightColor,1.0);
  	//vec4 vDirLightColor = GetDirectionalLightColor(sunLight, vNormalized);
    outputColor= vec4(lightColor, vTexColor.a);
}