#pragma once

#include "oglt_scene_node.h"

namespace oglt {
	namespace scene {

		enum LightParam {
			POSITION, DIRECTION, AMBIENT, DIFFUSE, SPECULAR,
			CUTOFF, OUTER_CUTOFF, CONSTANT, LINEAR, QUADRATIC
		};

		struct SpotLightParameter {
			glm::vec3 position;
			glm::vec3 direction;
			glm::vec3 ambient;
			glm::vec3 diffuse;
			glm::vec3 specular;
			glm::quat rotation;
			glm::vec3 eulerRotation;

			float cutOff;
			float outerCutOff;
			float constant;
			float linear;
			float quadratic;
			bool active;
		};

		struct DirectionalLightParameter {
			glm::vec3 position;
			glm::vec3 direction;
			glm::vec3 ambient;
			glm::vec3 diffuse;
			glm::vec3 specular;
			glm::quat rotation;
			glm::vec3 eulerRotation;
			bool active;
		};

		class DirectionalLight : public SceneNode {
		public:
			DirectionalLight();
			~DirectionalLight();
			void setParam(LightParam param, const glm::vec3& value);
			DirectionalLightParameter* getLightParameter();
			void update();
		private:
			DirectionalLightParameter parameter;
		};

		class SpotLight : public SceneNode {
		public:
			SpotLight();
			~SpotLight();
			void setParam(LightParam param, const glm::vec3& value);
			void setParam(LightParam param, const float value);
			SpotLightParameter* getLightParameter();
			void update();
		private:
			SpotLightParameter parameter;
		};
	}
}