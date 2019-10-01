#pragma once

#include "oglt_imaterial.h"

namespace oglt {
	class SkeletonMaterial : public IMaterial {
	public:
		SkeletonMaterial();
		~SkeletonMaterial();

		virtual void setColor(MaterialParam param, const glm::vec3& color);
		virtual void setFactor(MaterialParam param, float Factor);

		virtual glm::vec3* getColor(MaterialParam param);
		virtual float getFactor(MaterialParam param);
		virtual void linkTexture(MaterialParam param, uint textureId);
		virtual uint getTextureId(MaterialParam param);

		virtual void useMaterial();
	private:
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 emissive;
		float transparencyFactor;
		float shininessFactor;
		float reflectionFactor;

		bool enableAmbient;
		bool enableDiffuse;
		bool enableSpecular;
		bool enableEmissive;
		bool enableTransparencyFactor;
		bool enableShininessFactor;
		bool enableReflectionFactor;

		uint diffuseTextureId;
		uint specularTextureId;
		uint toonTextureId;
		uint cubeMapTextureId;
		uint furTextureId;
	};
}