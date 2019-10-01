#include "oglt_skeleton_material.h"

#include "oglt_irenderable.h"
#include "oglt_resource.h"

using namespace oglt;

SkeletonMaterial::SkeletonMaterial()
{
	enableAmbient = false;
	enableDiffuse = false;
	enableSpecular = false;
	enableEmissive = false;
	enableTransparencyFactor = false;
	enableShininessFactor = false;
	enableReflectionFactor = false;

	shaderProgram = NULL;

	diffuseTextureId = OGLT_INVALID_TEXTURE_INDEX;
	specularTextureId = OGLT_INVALID_TEXTURE_INDEX;
	toonTextureId = OGLT_INVALID_TEXTURE_INDEX;
	furTextureId = OGLT_INVALID_TEXTURE_ID;
}

SkeletonMaterial::~SkeletonMaterial()
{

}

void SkeletonMaterial::setColor(MaterialParam param, const glm::vec3& color)
{
	switch (param) {
	case AMBIENT:
		ambient = color;
		enableAmbient = true;
		break;
	case DIFFUSE:
		diffuse = color;
		enableDiffuse = true;
		break;
	case SPECULAR:
		specular = color;
		enableSpecular = true;
		break;
	case EMISSIVE:
		emissive = color;
		enableEmissive = true;
		break;
	default:
		fprintf(stderr, "Error: Use undefind \n");
	}
}

void SkeletonMaterial::setFactor(MaterialParam param, float factor)
{
	switch (param) {
	case TRANSPARENCY_FACTOR:
		transparencyFactor = factor;
		enableTransparencyFactor = true;
		break;
	case SHININESS_FACTOR:
		shininessFactor = factor;
		enableShininessFactor = true;
		break;
	case REFLECTION_FACTOR:
		reflectionFactor = factor;
		enableReflectionFactor = true;
		break;
	default:
		fprintf(stderr, "Error: Use undefined factor\n");
	}
}

glm::vec3 * oglt::SkeletonMaterial::getColor(MaterialParam param)
{
	switch (param) {
	case AMBIENT:
		if (enableAmbient)
			return &ambient;
		break;
	case DIFFUSE:
		if (enableDiffuse)
			return &diffuse;
		break;
	case SPECULAR:
		if (enableSpecular)
			return &specular;
		break;
	case EMISSIVE:
		if (enableEmissive)
			return &emissive;
		break;
	default:
		fprintf(stderr, "Error: Use undefind \n");
	}
	fprintf(stderr, "Error: Use unenable \n");
	return nullptr;
}

float SkeletonMaterial::getFactor(MaterialParam param)
{
	switch (param) {
	case TRANSPARENCY_FACTOR:
		if (enableTransparencyFactor)
			return transparencyFactor;
		break;
	case SHININESS_FACTOR:
		if (enableShininessFactor)
			return shininessFactor;
		break;
	case REFLECTION_FACTOR:
		if (enableReflectionFactor)
			return reflectionFactor;
		break;
	default:
		fprintf(stderr, "Error: Use undefined factor\n");
	}
	fprintf(stderr, "Error: Use unenable factor\n");
	return 0.0f;
}

void oglt::SkeletonMaterial::linkTexture(MaterialParam param, uint textureId)
{
	switch (param) {
	case DIFFUSE:
		diffuseTextureId = textureId;
		break;
	case SPECULAR:
		specularTextureId = textureId;
		break;
	case TOON:
		toonTextureId = textureId;
		break;
	case CUBE_MAP:
		cubeMapTextureId = textureId;
		break;
	case FUR_TEXTURE:
		furTextureId = textureId;
		break;
	default:
		fprintf(stderr, "Error: Use undefined texture param\n");
	}
}

uint oglt::SkeletonMaterial::getTextureId(MaterialParam param)
{
	switch (param) {
	case DIFFUSE:
		return diffuseTextureId;
	case SPECULAR:
		return specularTextureId;
	case TOON:
		return toonTextureId;
	case CUBE_MAP:
		return cubeMapTextureId;
	default:
		fprintf(stderr, "Error: Use undefined texture param\n");
	}
	return OGLT_INVALID_TEXTURE_INDEX;
}

void oglt::SkeletonMaterial::useMaterial()
{
	if (shaderProgram == NULL)
		return;

	if (shaderProgram != IRenderable::mutexShaderProgram) {
		shaderProgram->useProgram();
		shaderProgram->setUniform("matrices.viewMatrix", IRenderable::mutexViewMatrix);
		shaderProgram->setUniform("matrices.projMatrix", IRenderable::mutexProjMatrix);
		shaderProgram->setModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", IRenderable::mutexModelMatrix);
		shaderProgram->setUniform("sunLight.vColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shaderProgram->setUniform("sunLight.vDirection", IRenderable::mutexSunLightDir);
		shaderProgram->setUniform("sunLight.fAmbient", 1.0f);
		shaderProgram->setUniform("CameraPos", IRenderable::mutexCameraPos);
		shaderProgram->setUniform("gSampler", 0);
		shaderProgram->setUniform("envirMap", 1);
		shaderProgram->setUniform("furSampler", 2);

		if (IRenderable::mutexBoneTransforms != NULL) {
			vector<glm::mat4>* boneTransforms = IRenderable::mutexBoneTransforms;
			if (boneTransforms->size() > 0) {
				shaderProgram->setUniform("gBones", &boneTransforms->at(0), boneTransforms->size());
			}
		}
		IRenderable::mutexShaderProgram = shaderProgram;
	}

	if (enableAmbient)
		shaderProgram->setUniform("ambient", &ambient);
	if (enableDiffuse)
		shaderProgram->setUniform("diffuse", &diffuse);
	if (enableSpecular)
		shaderProgram->setUniform("specular", &specular);
	if (enableEmissive)
		shaderProgram->setUniform("emissive", &emissive);
	if (enableTransparencyFactor)
		shaderProgram->setUniform("transparencyFactor", transparencyFactor);
	if (enableShininessFactor)
		shaderProgram->setUniform("shininessFactor", shininessFactor);
	if (reflectionFactor)
		shaderProgram->setUniform("reflectionFactor", reflectionFactor);

	Texture* diffuseTexture = Resource::instance()->getTexture(diffuseTextureId);
	if (diffuseTexture != NULL) {
		diffuseTexture->bindTexture(0);
	}

	Texture* specularTexture = Resource::instance()->getTexture(specularTextureId);
	if (specularTexture != NULL) {
		specularTexture->bindTexture();
	}

	Texture* toonTexture = Resource::instance()->getTexture(toonTextureId);
	if (toonTexture != NULL) {
		toonTexture->bindTexture();
	}

	Texture* cubeMapTexture = Resource::instance()->getTexture(cubeMapTextureId);
	if (cubeMapTexture != NULL) {
		cubeMapTexture->bindTexture(1);
	}

	Texture* furTexture = Resource::instance()->getTexture(furTextureId);
	if (furTexture != NULL) {
		furTexture->bindTexture(2);
	}
}

