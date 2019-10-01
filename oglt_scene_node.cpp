#include "oglt_scene_node.h"

using namespace oglt::scene;

#include <glm/gtc/matrix_transform.hpp>

SceneNode::SceneNode()
{
	parent = NULL;
	child = NULL;
	brother = NULL;
	localTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	localTransform.rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
	localTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	modelMatrix = glm::mat4(1);
}

SceneNode::~SceneNode()
{
	
}

Transform* SceneNode::getLocalTransform()
{
	return &localTransform;
}

Transform* SceneNode::getWorldTransform()
{
	return &worldTransform;
}

glm::mat4 * SceneNode::getModelMatrix()
{
	return &modelMatrix;
}

void SceneNode::calcNodeHeirarchyTransform()
{
	if (parent != NULL) {
		worldTransform.position = parent->worldTransform.position 
			+ parent->worldTransform.rotation * localTransform.position;
		worldTransform.rotation = parent->worldTransform.rotation * localTransform.rotation;
		worldTransform.scale = parent->worldTransform.scale * localTransform.scale;
	}
	else {
		worldTransform = localTransform;
	}
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1), worldTransform.position);
	glm::mat4 rotationMatrix = glm::toMat4(worldTransform.rotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1), worldTransform.scale);
	modelMatrix = translateMatrix * rotationMatrix * scaleMatrix;
	
	if (brother != NULL)
		brother->calcNodeHeirarchyTransform();
	
	if (child != NULL)
		child->calcNodeHeirarchyTransform();
}

void SceneNode::addChild(SceneNode * child)
{
	if (child == NULL) return;

	if (this->child == NULL) {
		this->child = child;
	}
	else {
		SceneNode* lastBrother = this->child;
		while (lastBrother->brother != NULL) {
			lastBrother = lastBrother->brother;
		}
		lastBrother->brother = child;
	}
	child->parent = this;
}

void SceneNode::removeChild(SceneNode * child)
{
	if (child == NULL) return;

	SceneNode* node = this->child;
	SceneNode* preNode = NULL;
	while (node != child && node != NULL) {
		preNode = node;
		node = child->brother;
	}
	if (node == NULL) return;
	
	if (node->brother != NULL) {
		if (preNode != NULL) {
			preNode->brother = node->brother;
		}
		else {
			this->child = node->brother;
		}
	}
	else {
		if (preNode != NULL) {
			preNode->brother = NULL;
		}
		else {
			this->child = NULL;
		}
	}
}

bool SceneNode::isRootNode()
{
	return parent == NULL;
}

void SceneNode::render(int renderType)
{
	if (renderType & OGLT_RENDER_NONE)
		return;

	// the scene node has not any entity
	// ignore render it

	if (renderType & OGLT_RENDER_CHILDREN) {
		if (brother != NULL)
			brother->render(renderType);

		if (child != NULL)
			child->render(renderType);
	}
}
