#include "oglt_irenderable.h"

using namespace oglt;

uint IRenderable::mutexShaderProgramId = OGLT_INVALID_SHADER_ID;
glm::mat4* IRenderable::mutexModelMatrix;
glm::mat4 *IRenderable::mutexViewMatrix;
glm::mat4 *IRenderable::mutexProjMatrix;
glm::mat4 *IRenderable::mutexOrthoMatrix;
glm::vec3* IRenderable::mutexSunLightDir;
ShaderProgram* IRenderable::mutexShaderProgram;
vector<glm::mat4>* IRenderable::mutexBoneTransforms;
glm::vec3* IRenderable::mutexCameraPos;