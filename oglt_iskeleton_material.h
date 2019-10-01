#pragma once

#include "std_util.h"
#include "oglt_util.h"
#include <glm\glm.hpp>

namespace oglt {
	class ISkeletonMaterial {
	public:
		void setBoneTransforms(const vector<glm::mat4>& boneTransforms) {
			this->boneTransforms = boneTransforms;
		}
	protected:
		vector<glm::mat4> boneTransforms;
	};
}