#include "oglt_resource.h"

using namespace oglt;
using namespace oglt::scene;

Resource* Resource::instance()
{
	static Resource resource;
	return &resource;
}

void Resource::initialize()
{
	shaderPrograms.clear();
	textures.clear();
	materials.clear();

	Shader vtDefault, fgDefault;
	ShaderProgram spDefault;
	vtDefault.loadShader("data/shaders/main_shader.vert", GL_VERTEX_SHADER);
	fgDefault.loadShader("data/shaders/main_shader.frag", GL_FRAGMENT_SHADER);

	spDefault.createProgram();
	spDefault.addShaderToProgram(&vtDefault);
	spDefault.addShaderToProgram(&fgDefault);
	spDefault.linkProgram();

	shaderPrograms.push_back(spDefault);

	IMaterial* defMaterial = new IMaterial();
	defMaterial->setShaderProgram(&shaderPrograms[0]);

	materials.push_back(defMaterial);

	uboLights.createUniformBuffer();
}

uint Resource::addShaderProgram(ShaderProgram & shaderProgram)
{
	return addShaderProgram(shaderProgram, "");
}

uint oglt::Resource::addShaderProgram(const string & programName, int shaderCount, ...)
{
	ShaderProgram newProgram;
	newProgram.createProgram();
	va_list ap;
	va_start(ap, shaderCount);
	while (shaderCount--) {
		char* name = va_arg(ap, char*);
		string shaderName = name;
		Shader* shader = findShader(shaderName);
		if (shader != NULL) {
			newProgram.addShaderToProgram(shader);
		}
	}
	va_end(ap);
	newProgram.linkProgram();
	return addShaderProgram(newProgram, programName);
}

uint Resource::addShaderProgram(ShaderProgram & shaderProgram, const string& programName)
{
	// the shader program id store in Resource class
	uint shaderProgramId = OGLT_INVALID_SHADER_ID;
	FOR(i, ESZ(shaderPrograms)) {
		if (shaderProgram.getProgramID() == shaderPrograms[i].getProgramID()) {
			shaderProgramId = i;
			break;
		}
	}
	if (shaderProgramId == OGLT_INVALID_SHADER_ID) {
		shaderProgramId = ESZ(shaderPrograms);
		shaderPrograms.push_back(shaderProgram);
		string newProgramName = programName;
		if (newProgramName.size() == 0) {
			newProgramName = "ShaderProgram " + shaderProgramId;
		}
		shaderProgramMap[newProgramName] = shaderProgramId;
	}

	return shaderProgramId;
}

ShaderProgram * Resource::getShaderProgram(uint shaderId)
{
	if (shaderId == OGLT_INVALID_SHADER_ID || shaderId >= ESZ(shaderPrograms)) {
		return nullptr;
	}
	return &shaderPrograms[shaderId];
}

ShaderProgram * oglt::Resource::findShaderProgram(const string & programName)
{
	ShaderProgram* pShaderProgram = nullptr;
	if (shaderProgramMap.find(programName) != shaderProgramMap.end()) {
		pShaderProgram = getShaderProgram(shaderProgramMap[programName]);
	}
	return pShaderProgram;
}

uint oglt::Resource::addShader(Shader & shader)
{
	addShader(shader, "");
}

uint oglt::Resource::addShader(const string& shaderName, const string & filePath, int shaderType)
{
	Shader newShader;
	newShader.loadShader(filePath, shaderType);
	return addShader(newShader, shaderName);
}

uint oglt::Resource::addShader(Shader & shader, const string & shaderName)
{
	uint shaderId = OGLT_INVALID_SHADER_ID;
	FOR(i, ESZ(shaders)) {
		if (shader.getShaderID() == shaders[i].getShaderID()) {
			shaderId = i;
			break;
		}
	}

	if(shaderId == OGLT_INVALID_SHADER_ID){
		shaderId = ESZ(shaders);
		shaders.push_back(shader);
		string newShaderName = shaderName;
		if (newShaderName.size() == 0) {
			newShaderName = "shader " + shaderId;
		}
		shaderMap[newShaderName] = shaderId;
	}
	return shaderId;
}

Shader * oglt::Resource::getShader(uint shaderId)
{
	if (shaderId == OGLT_INVALID_SHADER_ID || shaderId >= shaders.size()) {
		return NULL;
	}
	return &shaders[shaderId];
}

Shader * oglt::Resource::findShader(const string & shaderName)
{
	if (shaderMap.find(shaderName) != shaderMap.end()) {
		return getShader(shaderMap[shaderName]);
	}
	return NULL;
}

uint Resource::addTexture(Texture & texture)
{
	uint textureId = OGLT_INVALID_TEXTURE_ID;
	FOR(i, ESZ(textures)) {
		if (texture.getPath() == textures[i].getPath()) {
			textureId = i;
			break;
		}
	}
	if (textureId == OGLT_INVALID_TEXTURE_ID) {
		textureId = ESZ(textures);
		textures.push_back(texture);
	}
	return textureId;
}

uint oglt::Resource::addTexture(const string & texturePath)
{
	uint textureId = OGLT_INVALID_TEXTURE_ID;
	FOR(i, ESZ(textures)) {
		if (texturePath == textures[i].getPath()) {
			textureId = i;
			break;
		}
	}
	if (textureId == OGLT_INVALID_TEXTURE_ID) {
		textureId = ESZ(textures);
		Texture newTexture;
		newTexture.loadTexture2D(texturePath, true);
		textures.push_back(newTexture);
	}
	return textureId;
}

Texture * Resource::getTexture(uint textureId)
{
	if (textureId == OGLT_INVALID_TEXTURE_ID || textureId >= ESZ(textures)) {
		return nullptr;
	}
	return &textures[textureId];
}

uint Resource::addMaterial(IMaterial* material)
{
	uint materialId = OGLT_INVALID_MATERIAL_ID;
	FOR(i, ESZ(materials)) {
		if (material == materials[i]) {
			materialId = i;
			break;
		}
	}
	if (materialId == OGLT_INVALID_MATERIAL_ID) {
		materialId = ESZ(materials);
		materials.push_back(material);
		materialMap[material->getName()] = materialId;
	}
	return materialId;
}

IMaterial* Resource::getMaterial(uint materialId)
{
	if (materialId == OGLT_INVALID_MATERIAL_ID || materialId >= ESZ(materials)) {
		return nullptr;
	}
	return materials[materialId];
}

IMaterial * oglt::Resource::findMaterial(const string & materialName)
{
	if (materialMap.find(materialName) != materialMap.end()) {
		return materials[materialMap[materialName]];
	}
	return nullptr;
}

uint oglt::Resource::addSkybox(const scene::Skybox& skybox)
{
	uint skyboxId = OGLT_INVALID_SKYBOX_ID;
	FOR(i, ESZ(skyboxes)) {
		if (skyboxes[i] == skybox) {
			skyboxId = i;
			break;
		}
	}
	if (skyboxId == OGLT_INVALID_SKYBOX_ID) {
		skyboxId = skyboxes.size();
		skyboxes.push_back(skybox);
	}
	return skyboxId;
}

scene::Skybox * oglt::Resource::getSkybox(uint skyboxId)
{
	if (skyboxId == OGLT_INVALID_SKYBOX_ID || skyboxId >= skyboxes.size()) {
		return nullptr;
	}
	return &skyboxes[skyboxId];
}

uint oglt::Resource::addDirectionalLight(const scene::DirectionalLight & directionalLight)
{
	return addDirectionalLight(directionalLight, "");
}

uint oglt::Resource::addDirectionalLight(const scene::DirectionalLight & directionalLight, const string & lightName)
{
	uint lightId = directionalLights.size();
	directionalLights.push_back(directionalLight);
	string newLightName = lightName;
	if (newLightName.size() == 0) {
		newLightName = "Directional Light " + lightId;
	}
	directionalLightMap[newLightName] = lightId;

	return lightId;
}

uint oglt::Resource::addSpotLight(const scene::SpotLight & spotLight)
{
	return addSpotLight(spotLight, "");
}

uint oglt::Resource::addSpotLight(const scene::SpotLight & spotLight, const string & lightName)
{
	uint lightId = spotLights.size();
	spotLights.push_back(spotLight);
	string newLightName = lightName;
	if (newLightName.size() == 0) {
		newLightName = "Spot Light " + lightId;
	}
	spotLightMap[newLightName] = lightId;

	return lightId;
}

DirectionalLight* oglt::Resource::getDirectionalLight(uint lightId)
{
	if (lightId == OGLT_INVALID_LIGHT_ID || lightId >= ESZ(directionalLights))
		return NULL;
	return &directionalLights[lightId];
}

DirectionalLight* oglt::Resource::findDirectionalLight(const string & lightName)
{
	if (directionalLightMap.find(lightName) != directionalLightMap.end()) {
		return &directionalLights[directionalLightMap[lightName]];
	}
	return NULL;
}

vector<scene::DirectionalLight>* oglt::Resource::getDirecationalLights()
{
	return &directionalLights;
}

SpotLight* oglt::Resource::getSpotLight(uint lightId)
{
	if (lightId == OGLT_INVALID_LIGHT_ID || lightId >= ESZ(spotLights))
		return NULL;
	return &spotLights[lightId];
}

SpotLight* oglt::Resource::findSpotLight(const string & lightName)
{
	if (spotLightMap.find(lightName) != spotLightMap.end()) {
		return &spotLights[spotLightMap[lightName]];
	}
	return NULL;
}

vector<scene::SpotLight>* oglt::Resource::getSpotLights()
{
	return &spotLights;
}

void oglt::Resource::setUpLights()
{
	int maxSize = MAX_DIRECTIONAL_LIGHTS;
	int currentSize = directionalLights.size();
	FOR(i, maxSize) {
		uboLights.addData(i < currentSize ? 
			&directionalLights[i].getWorldTransform()->position : NULL, sizeof(glm::vec4));
		uboLights.addData(i < currentSize ?
			&directionalLights[i].getLightParameter()->direction : NULL, sizeof(glm::vec4));
		uboLights.addData(i < currentSize ? 
			&directionalLights[i].getLightParameter()->ambient : NULL, sizeof(glm::vec4));
		uboLights.addData(i < currentSize ?
			&directionalLights[i].getLightParameter()->diffuse : NULL, sizeof(glm::vec4));
		uboLights.addData(i < currentSize ?
			&directionalLights[i].getLightParameter()->specular : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ?
			&directionalLights[i].getLightParameter()->active : NULL, sizeof(GLint));
	}

	maxSize = MAX_SPOT_LIGHTS;
	currentSize = spotLights.size();
	FOR(i, maxSize) {
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getWorldTransform()->position : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->cutOff : NULL, sizeof(GLfloat));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->direction : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->outerCutOff : NULL, sizeof(GLfloat));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->ambient : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->constant : NULL, sizeof(GLfloat));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->diffuse : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->linear : NULL, sizeof(GLfloat));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->specular : NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ? 
			&spotLights[i].getLightParameter()->quadratic : NULL, sizeof(GLfloat));
		// padding data
		uboLights.addData(NULL, sizeof(glm::vec3));
		uboLights.addData(i < currentSize ?
			&spotLights[i].getLightParameter()->active : NULL, sizeof(GLint));
	}

	directionalLightCount = directionalLights.size();
	spotLightCount = spotLights.size();

	uboLights.addData(&directionalLightCount, sizeof(GLint));
	uboLights.addData(&spotLightCount, sizeof(GLint));

	uboLights.uploadBufferData(1, GL_DYNAMIC_DRAW);
	uboLights.updateBuffer();
}

void oglt::Resource::updateLights()
{
    FOR(i, ESZ(directionalLights)) {
		directionalLights[i].update();
	}

	FOR(i, ESZ(spotLights)) {
		spotLights[i].update();
	}

	uboLights.updateBuffer();
}

Resource::Resource()
{
	initialize();
}

Resource::~Resource()
{
	FOR(i, ESZ(materials)) {
		delete materials[i];
	}

	FOR(i, ESZ(skyboxes)) {
		skyboxes[i].deleteSkybox();
	}

	FOR(i, ESZ(textures)) {
		textures[i].deleteTexture();
	}

	FOR(i, ESZ(shaderPrograms)) {
		shaderPrograms[i].deleteProgram();
	}

	FOR(i, ESZ(shaders)) {
		shaders[i].deleteShader();
	}

	uboLights.deleteBuffer();
}
