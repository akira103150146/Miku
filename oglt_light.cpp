#include "oglt_light.h"

using namespace oglt::scene;

DirectionalLight::DirectionalLight() {
	ZERO_MEM(&parameter);
	parameter.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	parameter.active = true;
}

DirectionalLight::~DirectionalLight() {
	
}

void DirectionalLight::setParam(LightParam param, const glm::vec3 & value)
{
	switch (param) {
	case POSITION:
		parameter.position = value;
		break;
	case DIRECTION:
		parameter.direction = value;
		break;
	case DIFFUSE:
		parameter.diffuse = value;
		break;
	case AMBIENT:
		parameter.ambient = value;
		break;
	case SPECULAR:
		parameter.specular = value;
		break;
	default:
		fprintf(stderr, "set the undefined value in DirectionalLight\n");
	}
}

DirectionalLightParameter* DirectionalLight::getLightParameter()
{
	return &parameter;
}

void DirectionalLight::update()
{
	parameter.direction = worldTransform.rotation * glm::vec3(0.0f, -1.0f, 0.0f);
	localTransform.rotation = glm::quat(parameter.eulerRotation);
	localTransform.position = parameter.position;
}

SpotLight::SpotLight()
{
	ZERO_MEM(&parameter);
	parameter.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	parameter.active = true;
}

SpotLight::~SpotLight()
{
}

void SpotLight::setParam(LightParam param, const glm::vec3 & value)
{
	switch (param) {
	case POSITION:
		parameter.position = value;
		break;
	case DIRECTION:
		parameter.direction = value;
		break;
	case DIFFUSE:
		parameter.diffuse = value;
		break;
	case AMBIENT:
		parameter.ambient = value;
		break;
	case SPECULAR:
		parameter.specular = value;
		break;
	default:
		fprintf(stderr, "set the undefined value in SpotLight\n");
	}
}

void SpotLight::setParam(LightParam param, const float value)
{
	switch(param) {
	case CUTOFF:
		parameter.cutOff = value;
		break;
	case OUTER_CUTOFF:
		parameter.outerCutOff = value;
		break;
	case CONSTANT:
		parameter.constant = value;
		break;
	case LINEAR:
		parameter.linear = value;
		break;
	case QUADRATIC:
		parameter.quadratic = value;
		break;
	default:
		fprintf(stderr, "set the undefined value in SpotLight\n");
	}
}

SpotLightParameter* SpotLight::getLightParameter()
{
	return &parameter;
}

void SpotLight::update()
{
	parameter.direction = worldTransform.rotation * glm::vec3(0.0f, -1.0f, 0.0f);
	localTransform.rotation = glm::quat(parameter.eulerRotation);
	localTransform.position = parameter.position;
}
