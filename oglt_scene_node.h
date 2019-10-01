#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include "oglt_irenderable.h"

namespace oglt {
	namespace scene {
		class SceneNode : public IRenderable {
		public:
			SceneNode();
			~SceneNode();
			Transform* getLocalTransform();
			Transform* getWorldTransform();
			glm::mat4* getModelMatrix();

			void calcNodeHeirarchyTransform();

			void addChild(SceneNode* child);
			void removeChild(SceneNode* child);
			bool isRootNode();

			virtual void render(int renderType = OGLT_RENDER_CHILDREN);

		protected:
			Transform localTransform;
			Transform worldTransform;

			glm::mat4 modelMatrix;

			SceneNode* parent;
			SceneNode* child;
			SceneNode* brother;
		};
	}
}