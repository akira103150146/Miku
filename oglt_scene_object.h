#pragma once

#include "oglt_util.h"
#include "std_util.h"
#include "oglt_scene_node.h"

namespace oglt {
	namespace scene {
		class SceneObject : public SceneNode {
		public:
			SceneObject();
			~SceneObject();

			void addRenderObj(oglt::IRenderable* renderObj);
			void removeRenderObj(oglt::IRenderable* renderObj);

			virtual void render(int renderType = OGLT_RENDER_SELF);
		private:
			vector<oglt::IRenderable*> renderObjs;
		};
	}
}