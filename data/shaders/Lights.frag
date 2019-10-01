#version 430 core

#include_part

struct Spotlight
{
	// fill to vec4
	vec3 position;
	float cutOff;

    vec3 direction;
	float outerCutOff;

    vec3 ambient;
	float constant;

    vec3 diffuse;
	float linear;

    vec3 specular; 
    float quadratic;

	vec3 padding;
	bool enable;
};
struct DirectionalLight
{
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	bool enable;
};

struct Material{

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

vec3 CaculSmoothSpotLight(vec3 WorldPos,vec3 Normal,vec3 cameraPos,Spotlight light,vec3 textureColor);
vec3 CaculDirectionalLightColor(vec3 WorldPos,vec3 Normal,vec3 CameraPos,DirectionalLight light, vec3 textureColor);
vec3 CaculSpotLightColor(vec3 WorldPos,vec3 Normal,vec3 cameraPos,Spotlight light,vec3 textureColor);
vec3 BlinnShading(vec3 textureColor,vec3 WorldPos,vec3 lightPos,vec3 CameraPos,vec3 Normal);
#definition_part

vec3 CaculDirectionalLightColor(vec3 WorldPos,vec3 Normal,vec3 CameraPos,DirectionalLight light, vec3 textureColor)
{

	vec3 norm=normalize(Normal);
	vec3 ambient=light.ambient*textureColor;

	vec3 lightDir=normalize(-light.direction);


	float diff=max(dot(norm,lightDir),0.0);
	vec3 diffuse=light.diffuse *diff*textureColor;

	vec3 viewDir= normalize(CameraPos-WorldPos);
	vec3 reflectDir = reflect(-lightDir,norm);
	float spec = pow(max(dot(viewDir,reflectDir),0.0),32.0);
	vec3 specular =light.specular * spec * textureColor;

	return vec3(ambient+diffuse+specular);
}

vec3 CaculSpotLightColor(vec3 WorldPos,vec3 Normal,vec3 cameraPos,Spotlight light,vec3 textureColor){

	vec3 lightDir=normalize(WorldPos-light.position);

	float theta=dot(normalize(light.direction),lightDir);

	if(theta > light.cutOff){
		// Ambient
        vec3 ambient = light.ambient * textureColor;

        // Diffuse 
        vec3 norm = normalize(Normal);        
        float diff = max(dot(-norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * textureColor;
        
        // Specular
        vec3 viewDir = normalize(cameraPos - WorldPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = light.specular * spec * textureColor;
        
        // Attenuation
        float Distance    = length(light.position - WorldPos);
        float attenuation = 1.0f / (light.constant + light.linear * Distance + light.quadratic * Distance );    

        // ambient  *= attenuation;  // Also remove attenuation from ambient, because if we move too far, the light in spotlight would then be darker than outside (since outside spotlight we have ambient lighting).
        //diffuse  *= attenuation;
        //specular *= attenuation;   
                
       	return  vec3(ambient + diffuse + specular); 
       	
	}
	else
	    return light.ambient *textureColor;



}
vec3 BlinnShading(vec3 textureColor,vec3 WorldPos,vec3 lightPos,vec3 CameraPos,vec3 Normal){
	vec3 ambient = 0.10*textureColor;

	vec3 lightDir=normalize(lightPos-WorldPos);
	vec3 normal=normalize(Normal);

	float diff= max(dot(lightDir,normal),0.0);
	vec3 diffuse = diff*textureColor;

	vec3 viewDir =normalize(CameraPos-WorldPos);
	float spec =0.0;

	vec3 halfwayDir=normalize(lightDir +viewDir);
	spec = pow(max(dot(normal,halfwayDir),0.0),32.0);

	vec3 specular =vec3(0.3)*spec;

	return vec3 (ambient+diffuse+specular);


}
vec3 CaculSmoothSpotLight(vec3 WorldPos,vec3 Normal,vec3 cameraPos,Spotlight light,vec3 textureColor){

		//Ambient
		vec3 ambient=light.ambient * textureColor *0.8;

		//Diffuse
		vec3 norm= normalize(Normal);
		vec3 lightDir = normalize(light.position-WorldPos);
		float diff=max(dot(norm,lightDir),0.0);
		vec3 diffuse =light.diffuse * diff * textureColor;

		//Specular
		vec3 viewDir = normalize(cameraPos - WorldPos);
		vec3 reflectDir = reflect (-lightDir,norm);
		float spec = pow(max(dot(viewDir,reflectDir),0.0),32.0);
		vec3 specular = light.specular*spec* vec3(0.3);

		//Spotlight(soft edge)
		float theta = dot(lightDir,normalize(-light.direction));
		float epsilon = light.cutOff -light.outerCutOff;
		//float epsilon = light.cutOff - light.cutOff;
		float intensity = clamp((theta-light.outerCutOff)/epsilon,0.0,1.0);
		//float intensity = clamp((theta-light.cutOff)/epsilon,0.0,1.0);
		diffuse *=intensity;
		specular *=intensity;

		float distance = length(light.position-WorldPos);
		float attenuation =1.0/ (light.constant + light.linear * distance + light.quadratic * (distance * distance));  

		//ambient  *= attenuation; 
    	//diffuse  *= attenuation;
    	//specular *= attenuation;   

    	return ambient+diffuse+specular;





}