#include "render_scene.h"

#include "oglt_util.h"
#include "std_util.h"

#include "oglt_shader.h"
#include "oglt_freetypefont.h"
#include "oglt_skybox.h"
#include "oglt_camera.h"
#include "oglt_assimp_model.h"
#include "oglt_fbx_model.h"
#include "oglt_scene_object.h"
#include "oglt_audio_source.h"
#include "oglt_light.h"

using namespace oglt;
using namespace oglt::scene;

using namespace glm;

#define SKYBOX_NUM 5
#define OPEN_LIGHT_SOURCE_NUM 3

FreeTypeFont ftFont;
FlyingCamera camera;
SceneObject worldTree, mikuObj, remObj, ramObj, iaxObj, stageObj;
FbxModel mikuModel, iaxModel, remModel, ramModel, stageModel;
bool enableMiku = true, enableIax = false, enableStage = true, enableRem = false, enableRam = false;

vec3 sunDir = vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0);

vector<oglt::uint> skyboxIds;
oglt::uint skyboxIndex;

vector<oglt::uint> cubeMapTextureIds;

int cameraUpdateMode = OGLT_UPDATEA_CAMERA_WALK | OGLT_UPDATE_CAMERA_ROTATE;

AudioSource tellYourWorldSource;
AudioSource kiminoSource;
AudioSource gokurakuSource;
AudioSource openLightSource[OPEN_LIGHT_SOURCE_NUM];

UniformBufferObject uboMatrix;

TwBar* bar;

void bindModelTextureId(FbxModel& fbxModel, MaterialParam textureType, oglt::uint textureId) {
	bool notTexture = textureType != MaterialParam::DIFFUSE && textureType != MaterialParam::CUBE_MAP
		&& textureType != MaterialParam::FUR_TEXTURE;
	if (notTexture) return;

	vector<Mesh>* meshs = fbxModel.getMeshs();
	if (meshs != NULL) {
		for (int i = 0; i < meshs->size(); i++) {
			if (meshs->at(i).mtlMapMode == ALL_SAME) {
				IMaterial* material = Resource::instance()->getMaterial(meshs->at(i).materialId);
				if (material != NULL) {
					material->linkTexture(textureType, textureId);
				}
			}
			else if (meshs->at(i).mtlMapMode == BY_POLYGON) {
				FOR(j, ESZ(meshs->at(i).polygons)) {
					IMaterial* material = Resource::instance()->getMaterial(meshs->at(i).polygons[j].materialId);
					if (material != NULL) {
						material->linkTexture(textureType, textureId);
					}
				}
			}
		}
	}
}

void initShaders() {
	Resource::instance()->addShader("ortho", "data/shaders/ortho2D.vert", GL_VERTEX_SHADER);
	Resource::instance()->addShader("fgFont", "data/shaders/font2D.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("vtMain", "data/shaders/main_shader.vert", GL_VERTEX_SHADER);
	Resource::instance()->addShader("fgMain", "data/shaders/main_shader.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgDirLight", "data/shaders/dirLight.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("vtSkin", "data/shaders/skinning_shader.vert", GL_VERTEX_SHADER);
	Resource::instance()->addShader("fgSkin", "data/shaders/skinning_shader.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("vtReflect", "data/shaders/reflect.vert", GL_VERTEX_SHADER);
	Resource::instance()->addShader("fgReflect", "data/shaders/reflect.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgHandPaint", "data/shaders/hand_painted_shader.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("vtFurSkin", "data/shaders/fur_skinning_shader.vert", GL_VERTEX_SHADER);
	Resource::instance()->addShader("geFurSkin", "data/shaders/fur_skinning_shader.geom", GL_GEOMETRY_SHADER);
	Resource::instance()->addShader("fgFurSkin", "data/shaders/fur_skinning_shader.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgLights", "data/shaders/Lights.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgLighting", "data/shaders/lighting.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgSharper", "data/shaders/sharper.frag", GL_FRAGMENT_SHADER);
	Resource::instance()->addShader("fgGaussionBlur", "data/shaders/gaussion_blur.frag", GL_FRAGMENT_SHADER);

	Resource::instance()->addShaderProgram("font", 2, "ortho", "fgFont");
	Resource::instance()->addShaderProgram("main", 3, "vtMain", "fgDirLight", "fgMain");
	Resource::instance()->addShaderProgram("skin", 3, "vtSkin", "fgDirLight", "fgSkin");
	Resource::instance()->addShaderProgram("reflect", 3, "vtReflect", "fgLights", "fgReflect");
	Resource::instance()->addShaderProgram("handedSkin", 3, "vtSkin", "fgDirLight", "fgHandPaint");
	Resource::instance()->addShaderProgram("furSkin", 3, "vtFurSkin", "geFurSkin", "fgFurSkin");
	Resource::instance()->addShaderProgram("lightingSkin", 3, "vtSkin", "fgLights", "fgLighting");
	Resource::instance()->addShaderProgram("skinSharper", 2, "vtSkin", "fgSharper");
	Resource::instance()->addShaderProgram("skinGaussionBlur", 2, "vtSkin", "fgGaussionBlur");
}

SceneObject spotLightBall;
vec3 eulerVector = vec3(0.0f);

void initLights() {
	DirectionalLight directionalLight;
	directionalLight.setParam(LightParam::AMBIENT, vec3(0.023f, 0.023f, 0.023f));
	directionalLight.setParam(LightParam::DIFFUSE, vec3(0.1f, 0.1f, 0.1f));
	directionalLight.setParam(LightParam::SPECULAR, vec3(0.0f, 0.0f, 0.0f));
	directionalLight.getLightParameter()->active = false;
	Resource::instance()->addDirectionalLight(directionalLight, "directionalLight1");
	Resource::instance()->addDirectionalLight(directionalLight, "directionalLight2");
	worldTree.addChild(Resource::instance()->findDirectionalLight("directionalLight1"));
	worldTree.addChild(Resource::instance()->findDirectionalLight("directionalLight2"));
	
	SpotLight spotLight;
	spotLight.setParam(LightParam::AMBIENT, vec3(0.0f, 0.0f, 0.0f));
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.0f, 0.0f, 0.8f));
	spotLight.setParam(LightParam::SPECULAR, vec3(1.0f, 1.0f, 1.0f));
	spotLight.setParam(LightParam::CONSTANT, 1.0f);
	spotLight.setParam(LightParam::LINEAR, 0.09f);
	spotLight.setParam(LightParam::QUADRATIC, 0.032f);
	spotLight.setParam(LightParam::CUTOFF, 0.61f);
	spotLight.setParam(LightParam::OUTER_CUTOFF, 0.5f);
	spotLight.getLightParameter()->position = vec3(0.0f, 0.0f, 0.0f);
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, 0.0f);
	spotLight.getLightParameter()->active = false;
	
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight1");
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.8f, 0.0f, 0.0f));
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, 90.0f);
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight2");
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.0f, 0.8f, 0.0f));
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, -90.0f);
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight3");
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.8f, 0.8f, 0.0f));
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, 180.0f);
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight4");
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.0f, 0.8f, 0.8f));
	spotLight.getLightParameter()->eulerRotation = vec3(90.0f, 0.0f, 0.0f);
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight5");
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.8f, 0.0f, 0.8f));
	spotLight.getLightParameter()->eulerRotation = vec3(-90.0f, 0.0f, 0.0f);
	Resource::instance()->addSpotLight(spotLight, "ball_1_spotLight6");

	spotLight.setParam(LightParam::DIFFUSE, vec3(0.8f, 0.8f, 0.8f));
	spotLight.setParam(LightParam::CUTOFF, 0.92f);
	spotLight.setParam(LightParam::OUTER_CUTOFF, 0.91f);
	spotLight.setParam(LightParam::SPECULAR, vec3(0.0f, 0.0f, 0.0f));
	spotLight.getLightParameter()->position = vec3(0.0f, 103.0f, 205.0f);
	spotLight.getLightParameter()->eulerRotation = vec3(1.25f, 0.0f, 0.0f);
	Resource::instance()->addSpotLight(spotLight, "worldSpotLight1");
	spotLight.getLightParameter()->position = vec3(208.0f, 235.0f, -11.0f);
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, -0.912f);
	Resource::instance()->addSpotLight(spotLight, "worldSpotLight2");
	spotLight.getLightParameter()->position = vec3(-216.0f, 235.0f, -11.0f);
	spotLight.getLightParameter()->eulerRotation = vec3(0.0f, 0.0f, 0.982f);
	Resource::instance()->addSpotLight(spotLight, "worldSpotLight3");
	spotLight.getLightParameter()->position = vec3(0.0f, 81.0f, 107.0f);
	spotLight.getLightParameter()->eulerRotation = vec3(1.21f, 0.0f, 0.0f);
	spotLight.setParam(LightParam::DIFFUSE, vec3(0.39f, 0.39f, 0.39f));
	Resource::instance()->addSpotLight(spotLight, "worldSpotLight4");
	Resource::instance()->setUpLights();

	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight1"));
	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight2"));
	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight3"));
	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight4"));
	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight5"));
	spotLightBall.addChild(Resource::instance()->findSpotLight("ball_1_spotLight6"));
	spotLightBall.getLocalTransform()->position = vec3(0.0f, 70.0f, 0.0f);
	stageObj.addChild(&spotLightBall);

	stageObj.addChild(Resource::instance()->findSpotLight("worldSpotLight1"));
	stageObj.addChild(Resource::instance()->findSpotLight("worldSpotLight2"));
	stageObj.addChild(Resource::instance()->findSpotLight("worldSpotLight3"));
	stageObj.addChild(Resource::instance()->findSpotLight("worldSpotLight4"));
}

void initTwBar() {
	bar = TwNewBar("TweakBar");
	TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLUT and OpenGL.' "); // Message added to the help bar.
	TwDefine(" TweakBar size='200 350' color='96 216 224'");
	TwStructMember positionMembers[] = {
		{ "X", TW_TYPE_FLOAT, offsetof(vec3, x), "" },
		{ "Y", TW_TYPE_FLOAT, offsetof(vec3, y), "" },
		{ "Z", TW_TYPE_FLOAT, offsetof(vec3, z), "" } };
	TwStructMember eulerMembers[] = {
		{ "X", TW_TYPE_FLOAT, offsetof(vec3, x), "step=0.002" },
		{ "Y", TW_TYPE_FLOAT, offsetof(vec3, y), "step=0.002" },
		{ "Z", TW_TYPE_FLOAT, offsetof(vec3, z), "step=0.002" } };
	TwStructMember scaleMembers[] = {
		{ "X", TW_TYPE_FLOAT, offsetof(vec3, x), "step=0.01" },
		{ "Y", TW_TYPE_FLOAT, offsetof(vec3, y), "step=0.01" },
		{ "Z", TW_TYPE_FLOAT, offsetof(vec3, z), "step=0.01" } };
	TwType positionType = TwDefineStruct("Position", positionMembers, 3, sizeof(vec3), NULL, NULL);
	TwType eulerType = TwDefineStruct("Euler", eulerMembers, 3, sizeof(vec3), NULL, NULL);
	TwType scaleType = TwDefineStruct("Scale", scaleMembers, 3, sizeof(vec3), NULL, NULL);
	TwStructMember directionalLightMembers[] = {
		{ "Active", TW_TYPE_BOOLCPP, offsetof(DirectionalLightParameter, active), ""},
		{ "Position", positionType, offsetof(DirectionalLightParameter, position), ""},
		{ "Rotation", eulerType, offsetof(DirectionalLightParameter, eulerRotation), ""},
		{ "Ambient Color", TW_TYPE_COLOR3F, offsetof(DirectionalLightParameter, ambient), ""},
		{ "Diffuse Color", TW_TYPE_COLOR3F, offsetof(DirectionalLightParameter, diffuse), ""},
		{ "Specular Color", TW_TYPE_COLOR3F, offsetof(DirectionalLightParameter, specular), ""}
	};
	
	TwStructMember spotLightMembers[] = {
		{ "Active", TW_TYPE_BOOLCPP, offsetof(SpotLightParameter, active), "" },
		{ "Position", positionType, offsetof(SpotLightParameter, position), "" },
		{ "Rotation", eulerType, offsetof(SpotLightParameter, eulerRotation), "" },
		{ "Ambient Color", TW_TYPE_COLOR3F, offsetof(SpotLightParameter, ambient), "" },
		{ "Diffuse Color", TW_TYPE_COLOR3F, offsetof(SpotLightParameter, diffuse), "" },
		{ "Specular Color", TW_TYPE_COLOR3F, offsetof(SpotLightParameter, specular), "" },
		{ "Cutoff", TW_TYPE_FLOAT, offsetof(SpotLightParameter, cutOff), "step=0.001"},
		{ "Out Cutoff", TW_TYPE_FLOAT, offsetof(SpotLightParameter, outerCutOff), "step=0.001"},
		{ "Constant", TW_TYPE_FLOAT, offsetof(SpotLightParameter, constant), "step=0.001"},
		{ "Linear", TW_TYPE_FLOAT, offsetof(SpotLightParameter, linear), "step=0.001" },
		{ "quadratic", TW_TYPE_FLOAT, offsetof(SpotLightParameter, quadratic), "step=0.001" }
	};

	TwStructMember transformMembers[] = {
		{ "Position", positionType, offsetof(Transform, position), "" },
		{ "Rotation", TW_TYPE_QUAT4F, offsetof(Transform, rotation), "" },
		{ "Scale", scaleType, offsetof(Transform, scale), ""}
	};

	TwType directionalLightType = TwDefineStruct("Directional Light", directionalLightMembers, 5, sizeof(DirectionalLightParameter), NULL, NULL);
	TwType spotLightType = TwDefineStruct("Spot Light", spotLightMembers, 11, sizeof(SpotLightParameter), NULL, NULL);
	TwType transformType = TwDefineStruct("Transform", transformMembers, 3, sizeof(Transform), NULL, NULL);

	DirectionalLight* directinalLight = Resource::instance()->findDirectionalLight("directionalLight1");
	if (directinalLight != NULL) {
		TwAddVarRW(bar, "DirectionalLight1", directionalLightType, directinalLight->getLightParameter(), "");
	}
	directinalLight = Resource::instance()->findDirectionalLight("directionalLight2");
	if (directinalLight != NULL) {
		TwAddVarRW(bar, "DirectionalLight2", directionalLightType, directinalLight->getLightParameter(), "");
	}

	TwAddVarRW(bar, "Miku Active", TW_TYPE_BOOLCPP, &enableMiku, "Group='Miku', Label='Active'");
	TwAddVarRW(bar, "Miku Transform", transformType, mikuObj.getLocalTransform(), "Group='Miku', Label='Transform'");
	TwAddVarRW(bar, "IAx Active", TW_TYPE_BOOLCPP, &enableIax, "Group='IAx', Label='Active'");
	TwAddVarRW(bar, "IAx Transform", transformType, iaxObj.getLocalTransform(), "Group='IAx', Label='Transform'");
	TwAddVarRW(bar, "Rem Active", TW_TYPE_BOOLCPP, &enableRem, "Group='Rem', Label='Active'");
	TwAddVarRW(bar, "Rem Transform", transformType, remObj.getLocalTransform(), "Group='Rem', Label='Transform'");
	TwAddVarRW(bar, "Ram Active", TW_TYPE_BOOLCPP, &enableRam, "Group='Ram', Label='Active'");
	TwAddVarRW(bar, "Ram Transform", transformType, ramObj.getLocalTransform(), "Group='Ram', Label='Transform'");
	TwAddVarRW(bar, "Stage Active", TW_TYPE_BOOLCPP, &enableStage, "Group='Stage', Label='Active'");
	TwAddVarRW(bar, "Stage Transform", transformType, stageObj.getLocalTransform(), "Group='Stage', Label='Transform'");
	TwAddVarRW(bar, "SpotLightBall_1_Pos", positionType, &spotLightBall.getLocalTransform()->position, "Group='SpotLightBall_1' Label='Position'");
	TwAddVarRW(bar, "SpotLightBall_1_Rot", eulerType, &eulerVector, "Group='SpotLightBall_1' Label='Rotate'");
	TwAddVarRW(bar, "SpotLight_1", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight1")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight1'");
	TwAddVarRW(bar, "SpotLight_2", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight2")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight2'");
	TwAddVarRW(bar, "SpotLight_3", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight3")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight3'");
	TwAddVarRW(bar, "SpotLight_4", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight4")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight4'");
	TwAddVarRW(bar, "SpotLight_5", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight5")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight5'");
	TwAddVarRW(bar, "SpotLight_6", spotLightType, Resource::instance()->findSpotLight("ball_1_spotLight6")->getLightParameter(), "Group='SpotLightBall_1' Label='SpotLight6'");
	TwAddVarRW(bar, "World Spot Light1", spotLightType, Resource::instance()->findSpotLight("worldSpotLight1")->getLightParameter(), "");
	TwAddVarRW(bar, "World Spot Light2", spotLightType, Resource::instance()->findSpotLight("worldSpotLight2")->getLightParameter(), "");
	TwAddVarRW(bar, "World Spot Light3", spotLightType, Resource::instance()->findSpotLight("worldSpotLight3")->getLightParameter(), "");
	TwAddVarRW(bar, "World Spot Light4", spotLightType, Resource::instance()->findSpotLight("worldSpotLight4")->getLightParameter(), "");
}

void initSkyboxes() {
	string skyboxPaths[SKYBOX_NUM * 7] = { "data/skyboxes/elbrus/" , "elbrus_front.jpg" , "elbrus_back.jpg" , "elbrus_right.jpg",
		"elbrus_left.jpg" , "elbrus_top.jpg", "elbrus_top.jpg" ,
		"data/skyboxes/jajlands1/", "jajlands1_ft.jpg", "jajlands1_bk.jpg", "jajlands1_lf.jpg",
		"jajlands1_rt.jpg", "jajlands1_up.jpg", "jajlands1_dn.jpg",
		"data/skyboxes/jf_nuke/", "nuke_ft.tga", "nuke_bk.tga", "nuke_lf.tga",
		"nuke_rt.tga", "nuke_up.tga", "nuke_dn.tga",
		"data/skyboxes/sor_borg/", "borg_ft.jpg", "borg_bk.jpg",
		"borg_lf.jpg", "borg_rt.jpg", "borg_up.jpg", "borg_up.jpg" ,
		"data/skyboxes/sor_cwd/", "cwd_ft.jpg", "cwd_bk.jpg", "cwd_lf.jpg",
		"cwd_lf.jpg", "cwd_up.jpg", "cwd_dn.jpg" };

	FOR(i, SKYBOX_NUM) {
		Skybox skybox;
		skybox.load(skyboxPaths[i * 7], skyboxPaths[i * 7 + 1], skyboxPaths[i * 7 + 2], skyboxPaths[i * 7 + 3], skyboxPaths[i * 7 + 4], skyboxPaths[i * 7 + 5], skyboxPaths[i * 7 + 6]);
		skybox.setShaderProgram(Resource::instance()->findShaderProgram("main"));
		skybox.getLocalTransform()->scale = vec3(10.0f, 10.0f, 10.0f);
		oglt::uint skyboxId = Resource::instance()->addSkybox(skybox);
		skyboxIds.push_back(skyboxId);

		Texture cubeMapTexture;
		cubeMapTexture.loadTexture(skyboxPaths[i * 7] + skyboxPaths[i * 7 + 1], skyboxPaths[i * 7] + skyboxPaths[i * 7 + 2],
			skyboxPaths[i * 7] + skyboxPaths[i * 7 + 4], skyboxPaths[i * 7] + skyboxPaths[i * 7 + 3], skyboxPaths[i * 7] + skyboxPaths[i * 7 + 5], skyboxPaths[i * 7] + skyboxPaths[i * 7 + 6]);
		oglt::uint cubeMapId = Resource::instance()->addTexture(cubeMapTexture);
		cubeMapTextureIds.push_back(cubeMapId);
	}

	skyboxIndex = skyboxIds.size() - 1;
}

void initSceneObjects(IApp* app) {
	camera = FlyingCamera(app, vec3(-6.75f, 137.9f, 315.29f), vec3(52.05f, -2.66f, -141.48f), vec3(0.0f, 1.0f, 0.0f), 100.0f, 0.01f);
	camera.setMovingKeys('w', 's', 'a', 'd');
	camera.addChild(Resource::instance()->getSkybox(skyboxIndex));

	FbxModel::initialize();

	worldTree.addChild(&camera);

	stageModel.load("data/models/Rurusyu/scenes/rurusyu.fbx");
	stageObj.addRenderObj(&stageModel);
	stageObj.setShaderProgram(Resource::instance()->findShaderProgram("lightingSkin"));
	worldTree.addChild(&stageObj);

	mikuModel.load("data/models/TdaJKStyleMaya2/scenes/TdaJKStyle.fbx");
	mikuObj.addRenderObj(&mikuModel);
	mikuObj.setShaderProgram(Resource::instance()->findShaderProgram("lightingSkin"));
	mikuObj.getLocalTransform()->scale = vec3(0.75f, 0.75f, 0.75f);
	stageObj.addChild(&mikuObj);

	iaxModel.load("data/models/IAxMaya2/scenes/IAx.fbx");
	iaxObj.addRenderObj(&iaxModel);
	iaxObj.setShaderProgram(Resource::instance()->findShaderProgram("lightingSkin"));
	iaxObj.getLocalTransform()->scale = vec3(0.75f, 0.75f, 0.75f);
	stageObj.addChild(&iaxObj);

	IRenderable::mutexViewMatrix = camera.look();
	IRenderable::mutexProjMatrix = app->getProj();
	IRenderable::mutexOrthoMatrix = app->getOrth();
	IRenderable::mutexSunLightDir = &sunDir;
	IRenderable::mutexCameraPos = &camera.getWorldTransform()->position;

	// the skybox index is same as cube map texture index
	bindModelTextureId(stageModel, MaterialParam::CUBE_MAP, cubeMapTextureIds[skyboxIndex]);
	bindModelTextureId(mikuModel, MaterialParam::CUBE_MAP, cubeMapTextureIds[skyboxIndex]);

	Texture furTexture;
	furTexture.loadTexture2D("data/textures/furFill.png");
	oglt::uint furTextureId = Resource::instance()->addTexture(furTexture);
	bindModelTextureId(mikuModel, MaterialParam::FUR_TEXTURE, furTextureId);
}

void initAudioSource() {
	tellYourWorldSource.load("data/musics/Tell Your World Dance.wav");
	kiminoSource.load("data/musics/kimino.wav");
	gokurakuSource.load("data/musics/gokuraku.wav");
	for (int i = 0; i < OPEN_LIGHT_SOURCE_NUM; i++) {
		openLightSource[i].load("data/musics/meka_ge_shoumei_swi01.wav");
	}
}

void scene::initScene(oglt::IApp* app) {
	glClearColor(0.1f, 0.3f, 0.7f, 1.0f);

	initShaders();

	ftFont.loadFont("data/fonts/SugarpunchDEMO.otf", 32);
	ftFont.setShaderProgram(Resource::instance()->findShaderProgram("font"));

	initSkyboxes();

	initSceneObjects(app);

	initLights();

	initTwBar();
	
	initAudioSource();

	uboMatrix.createUniformBuffer();
	uboMatrix.addData(IRenderable::mutexProjMatrix, sizeof(glm::mat4));
	uboMatrix.addData(IRenderable::mutexViewMatrix, sizeof(glm::mat4));
	uboMatrix.uploadBufferData(0, GL_DYNAMIC_DRAW);
	uboMatrix.updateBuffer();

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
}

float animTimer = 0.0f;
float lightingTimer = 0.0f;
float rotateLightBallY = 0.0f;
bool playAnimation = false;
bool playLightSource = false;
int openLightFrag = 0;
bool showInformation = false;
int switchShaderProgram = 0;

void updateSwitchShader(IApp* app) {
	if (app->oneKey('e') || app->oneKey('E')) {
		switchShaderProgram++;
		if (switchShaderProgram >= 5) {
			switchShaderProgram = 0;
		}
		ShaderProgram* program = nullptr;
		switch (switchShaderProgram) {
		case 0:
			program = Resource::instance()->findShaderProgram("skin");
			break;
		case 1:
			program = Resource::instance()->findShaderProgram("handedSkin");
			break;
		case 2:
			program = Resource::instance()->findShaderProgram("lightingSkin");
			break;
		case 3:
			program = Resource::instance()->findShaderProgram("skinSharper");
			break;
		case 4:
			program = Resource::instance()->findShaderProgram("skinGaussionBlur");
			break;
		}
		if (program != nullptr) {
			mikuModel.setShaderProgram(program);
			stageModel.setShaderProgram(program);
			iaxModel.setShaderProgram(program);
		}
	}
}

void updateAnim(IApp* app) {
	if (app->oneKey('t') || app->oneKey('T')) {
		lightingTimer = 0.0f;
		mikuModel.setTimer(-0.4f);
		tellYourWorldSource.rewind();
		tellYourWorldSource.play();
		//gokurakuSource.rewind();
		//gokurakuSource.play();
		playLightSource = true;
		openLightFrag = 0;
		eulerVector = vec3(0.0f, 0.0f, 0.0f);
		Resource::instance()->findSpotLight("worldSpotLight1")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("worldSpotLight2")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("worldSpotLight3")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("worldSpotLight4")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight1")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight2")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight3")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight4")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight5")->getLightParameter()->active = false;
		Resource::instance()->findSpotLight("ball_1_spotLight6")->getLightParameter()->active = false;
		Resource::instance()->findDirectionalLight("directionalLight1")->getLightParameter()->active = false;
		mikuModel.setShaderProgram(Resource::instance()->findShaderProgram("lightingSkin"));
		stageModel.setShaderProgram(Resource::instance()->findShaderProgram("lightingSkin"));
		camera.removeChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
		skyboxIndex = 4;
		camera.addChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
	}

	if (app->oneKey('y') || app->oneKey('Y')) {
		playAnimation = !playAnimation;
	}

	if (playAnimation) {
		animTimer += app->getDeltaTime();
		lightingTimer += app->getDeltaTime();
		if (animTimer >= 0.04f) {
			mikuModel.updateAnimation(animTimer);
			iaxModel.updateAnimation(animTimer);
			remModel.updateAnimation(animTimer);
			ramModel.updateAnimation(animTimer);
			animTimer = 0.0f;
		}
		if (playLightSource) {
			if (lightingTimer > 0.4f) {
				openLightSource[0].play();
			}
			if (lightingTimer > 0.9f) {
				openLightSource[1].play();
			}
			if (lightingTimer > 1.4f) {
				openLightSource[2].play();
				playLightSource = false;
			}
		}


		if (openLightFrag == 0) {
			if (lightingTimer > 0.5f) {
				Resource::instance()->findSpotLight("worldSpotLight1")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("worldSpotLight4")->getLightParameter()->active = true;
			}

			if (lightingTimer > 1.0f) {
				Resource::instance()->findSpotLight("worldSpotLight2")->getLightParameter()->active = true;
			}

			if (lightingTimer > 1.5f) {
				Resource::instance()->findSpotLight("worldSpotLight3")->getLightParameter()->active = true;
			}

			if (lightingTimer > 1.7f) {
				Resource::instance()->findDirectionalLight("directionalLight1")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight5")->getLightParameter()->active = true;
				openLightFrag = 1;
			}
		}

		if (openLightFrag == 1) {
			if (lightingTimer > 15.0f) {
				Resource::instance()->findSpotLight("worldSpotLight1")->getLightParameter()->active = false;
				Resource::instance()->findSpotLight("worldSpotLight2")->getLightParameter()->active = false;
				Resource::instance()->findSpotLight("worldSpotLight3")->getLightParameter()->active = false;
				Resource::instance()->findSpotLight("worldSpotLight4")->getLightParameter()->active = false;
				Resource::instance()->findSpotLight("ball_1_spotLight1")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight2")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight3")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight4")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight5")->getLightParameter()->active = true;
				Resource::instance()->findSpotLight("ball_1_spotLight6")->getLightParameter()->active = true;
				mikuModel.setShaderProgram(Resource::instance()->findShaderProgram("skin"));
				camera.removeChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
				skyboxIndex = 3;
				camera.addChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
				openLightFrag = 2;
			}
		}

		if (openLightFrag == 2) {
			rotateLightBallY += 3.0f * app->getDeltaTime();
			eulerVector = vec3(0.0f, rotateLightBallY, 0.0f);

			if (lightingTimer > 30.0f) {
				stageModel.setShaderProgram(Resource::instance()->findShaderProgram("reflect"));
				openLightFrag = 3;
			}
		}

		if (openLightFrag == 3) {
			rotateLightBallY += 3.0f * app->getDeltaTime();
			eulerVector = vec3(0.0f, rotateLightBallY, 0.0f);
			if (lightingTimer > 45.0f) {
				camera.removeChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
				skyboxIndex = 1;
				camera.addChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
				mikuModel.setShaderProgram(Resource::instance()->findShaderProgram("handedSkin"));
				stageModel.setShaderProgram(Resource::instance()->findShaderProgram("handedSkin"));
				openLightFrag = 4;
			}
		}

		if (openLightFrag == 4) {
			if (lightingTimer > 64.0f) {
				enableMiku = false;
				enableIax = true;
				iaxObj.getLocalTransform()->position = vec3(0.0f, -200.0f, 0.0f);
				iaxModel.setTimer(0.0f);
				iaxModel.updateAnimation(0.0f);
				iaxModel.setShaderProgram(Resource::instance()->findShaderProgram("handedSkin"));
				openLightFrag = 5;
			}
		}

		if (openLightFrag == 5) {
			if (iaxObj.getLocalTransform()->position.y < 0.0f) {
				iaxObj.getLocalTransform()->position.y += 30.0f * app->getDeltaTime();
			}
			animTimer = 0.0f;
			if (lightingTimer > 74.0f) {
				kiminoSource.rewind();
				kiminoSource.play();
				openLightFrag = 7;
			}
		}

		if (openLightFrag == 7 && lightingTimer > 89.0f) {
			iaxModel.setShaderProgram(Resource::instance()->findShaderProgram("skinSharper"));
			stageModel.setShaderProgram(Resource::instance()->findShaderProgram("skinSharper"));
			openLightFrag = 8;
		}

		if (openLightFrag == 8 && lightingTimer > 105.0f) {
			iaxModel.setShaderProgram(Resource::instance()->findShaderProgram("skinGaussionBlur"));
			stageModel.setShaderProgram(Resource::instance()->findShaderProgram("skinGaussionBlur"));
			openLightFrag = 9;
		}

		if (openLightFrag == 9 && lightingTimer > 120.0f) {
			iaxModel.setShaderProgram(Resource::instance()->findShaderProgram("skin"));
			stageModel.setShaderProgram(Resource::instance()->findShaderProgram("skin"));
			openLightFrag = 10;
		}
	}
}

void updateCameraMode(IApp* app) {
	if (app->oneKey('r') || app->oneKey('R')) {
		if (cameraUpdateMode & OGLT_UPDATE_CAMERA_ROTATE) {
			cameraUpdateMode ^= OGLT_UPDATE_CAMERA_ROTATE;
			app->setCursor(OGLT_CURSOR_ARROW);
		}
		else {
			cameraUpdateMode |= OGLT_UPDATE_CAMERA_ROTATE;
			app->setCursor(OGLT_CURSOR_NONE);
		}
	}
}

void oglt::scene::updateScene(IApp * app)
{
	worldTree.calcNodeHeirarchyTransform();

	updateCameraMode(app);
	updateSwitchShader(app);
	updateAnim(app);

	spotLightBall.getLocalTransform()->rotation = quat(eulerVector);
	stageObj.setVisiable(enableStage);
	mikuObj.setVisiable(enableMiku);
	iaxObj.setVisiable(enableIax);
	remObj.setVisiable(enableRem);
	ramObj.setVisiable(enableRam);

	if (app->oneKey('o')) {
		showInformation = !showInformation;
	}

	if (app->oneKey('i')) {
		openLightFrag = 99;
		iaxModel.setTimer(0.0f);
		mikuModel.setTimer(0.0f);
	}
}

void renderFont(IApp* app) {
	ShaderProgram* spFont = Resource::instance()->findShaderProgram("font");

	if (spFont != NULL) {
		spFont->useProgram();
		spFont->setUniform("matrices.projMatrix", app->getOrth());
		spFont->setUniform("vColor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	oglt::uint w, h;
	app->getViewport(w, h);
	glDisable(GL_DEPTH_TEST);
	if (showInformation) {
		ftFont.printFormatted(20, h - 35, 24, "UPS: %d", app->getUps());
		ftFont.printFormatted(20, h - 70, 24, "FPS: %d", app->getFps());
		ftFont.printFormatted(20, h - 100, 20, "PX: %.2f", camera.getWorldTransform()->position.x);
		ftFont.printFormatted(20, h - 123, 20, "PY: %.2f", camera.getWorldTransform()->position.y);
		ftFont.printFormatted(20, h - 146, 20, "PZ: %.2f", camera.getWorldTransform()->position.z);

		ftFont.printFormatted(20, h - 169, 20, "VX: %.2f", camera.getView()->x);
		ftFont.printFormatted(20, h - 192, 20, "VY: %.2f", camera.getView()->y);
		ftFont.printFormatted(20, h - 215, 20, "VZ: %.2f", camera.getView()->z);
		ftFont.print("OgltApp : https://github.com/Lei-k/oglt_app", 10, 15, 20);
		ftFont.render();
	}
	else {
		if (playAnimation) {
			switch (openLightFrag) {
			case 1:
				ftFont.print("ALL Lighting", 20, h - 45, 26);
				break;
			case 2:
				ftFont.print("Original Miku + Lighting Ball", 20, h - 45, 26);
				break;
			case 3:
				ftFont.print("Original Miku + Reflect Stage", 20, h - 45, 26);
				break;
			case 4: case 5: case 6: case 7:
				ftFont.print("Handed", 20, h - 45, 26);
				break;
			case 8:
				ftFont.print("Sharper", 20, h - 45, 26);
				break;
			case 9:
				ftFont.print("Gaussion Bulr", 20, h - 45, 26);
				break;
			case 10:
				ftFont.print("Original IAx", 20, h - 45, 26);
				break;
			default:
				break;
			}
			ftFont.render();
		}
	}
	glEnable(GL_DEPTH_TEST);
}

void scene::renderScene(oglt::IApp* app) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uboMatrix.updateBuffer();
	Resource::instance()->updateLights();

	// I put the camera update function in render scene
	// because in the camera update function I use the app->getFrameDeltaTime
	// function it's return delta time of per render scene.
	// it's more smooth then delta time. the camera is sensitive so I do that in
	// render scene. rather then in update scene.
	camera.update(cameraUpdateMode);

	worldTree.render(OGLT_RENDER_CHILDREN);

	renderFont(app);

	// I put change skybox code in render scene
	// because render scene and update scene are
	// called by difference threads.
	// if I remove some thing in world three
	// in update scene. it may cause nullptr problem when
	// render scene render some thing in world tree
	// and this operation is not weighted, so it's ok to put it
	// in render scene
	if (app->oneKey('p')) {
		camera.removeChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
		if (skyboxIndex < skyboxIds.size() - 1) {
			skyboxIndex++;
		}
		else {
			skyboxIndex = 0;
		}
		camera.addChild(Resource::instance()->getSkybox(skyboxIds[skyboxIndex]));
		bindModelTextureId(stageModel, CUBE_MAP, cubeMapTextureIds[skyboxIndex]);
		bindModelTextureId(mikuModel, CUBE_MAP, cubeMapTextureIds[skyboxIndex]);
	}

	if (!(cameraUpdateMode & OGLT_UPDATE_CAMERA_ROTATE)) {
		TwDraw();
	}

	app->swapBuffers();
}

void scene::releaseScene(oglt::IApp* app) {
	FbxModel::destroyManager();

	TwDeleteBar(bar);
}
