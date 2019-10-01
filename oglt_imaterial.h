#pragma once

#include "std_util.h"
#include "oglt_util.h"
#include "oglt_texture.h"
#include "oglt_shader.h"
#include "oglt_ishaderable.h"
#include <glm/glm.hpp>

namespace oglt {
	enum MaterialParam {
		AMBIENT, DIFFUSE, SPECULAR, EMISSIVE,
		TRANSPARENCY_FACTOR, SHININESS_FACTOR, REFLECTION_FACTOR,
		TOON, CUBE_MAP, FUR_TEXTURE
	};

	class IMaterial : public IShaderable {
	public:
		void setName(const string& materialName) {
			name = materialName;
		}

		string getName() {
			return name;
		}

		virtual	void setColor(MaterialParam param, const glm::vec3& color) {}
		virtual glm::vec3* getColor(MaterialParam param) { return nullptr; }
		virtual void setFactor(MaterialParam param, float factor) {}
		virtual float getFactor(MaterialParam param) { return 0.0f; }
		virtual void linkTexture(MaterialParam param, uint textureId) {}
		virtual uint getTextureId(MaterialParam param) { return OGLT_INVALID_TEXTURE_ID; }

		virtual void useMaterial() {}
	protected:
		string name;
	};
}