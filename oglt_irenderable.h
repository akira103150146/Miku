#pragma once

#include "oglt_util.h"
#include "oglt_shader.h"
#include "oglt_ishaderable.h"

namespace oglt {
	class IRenderable : public IShaderable {
	public:
		virtual void render(int renderType = OGLT_RENDER_SELF) {}
		void setVisiable(bool _visiable) {
			visiable = _visiable;
		}

		static uint mutexShaderProgramId;
		static glm::mat4* mutexModelMatrix;
		static glm::mat4* mutexViewMatrix;
		static glm::mat4* mutexProjMatrix;
		static glm::mat4* mutexOrthoMatrix;
		static glm::vec3* mutexSunLightDir;
		static ShaderProgram* mutexShaderProgram;
		static vector<glm::mat4>* mutexBoneTransforms;
		static glm::vec3* mutexCameraPos;

	protected:
		bool visiable = true;
	};
}