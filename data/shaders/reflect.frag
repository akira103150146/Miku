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
uniform int enableLight;
uniform float reflectMixFactor = 0.6;
Material material;

void main()
{
	
	//###Region akira
	vec3 ViewVector=normalize (vWorldPos-CameraPos);
	vec3 unitNormal=normalize(vNormal);

	//vec3 Reflect_Vector= reflect(ViewVector,unitNormal);
	//vec4 Reflect_Color=texture(envirMap, Reflect_Vector);
	vec3 Refract_Vector= refract(ViewVector,WorldNormal,1.0/1.33);
	vec4 Refract_Color=texture(envirMap,Refract_Vector);
	//####Region akira

	
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = texture2D(gSampler, vTexCoord);
	vec3 vTexColorRgb = vTexColor.rgb;
	//CaculateLights
	vec3 lightColor=vec3(0.0,0.0,0.0);
	
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
	//vec3 lightpos=vec3(0.0,5.0,0.0);
	//vec3 blinColor = BlinnShading(vTexColorRgb,vWorldPos,lightpos,CameraPos,unitNormal);

	
	//vMixedColor+=lightColor;
	vec4 totalColor= vec4(lightColor,1.0);
  	//vec4 vDirLightColor = GetDirectionalLightColor(sunLight, vNormalized);
	outputColor = mix(totalColor,Refract_Color, reflectMixFactor); //mix color by 1:1 You can change 												//transparency by change the float value
}