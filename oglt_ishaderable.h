#pragma once

#include "oglt_shader.h"

namespace oglt {
	class IShaderable {
	public:
		void setShaderProgram(ShaderProgram* _shaderProgram) {
			shaderProgram = _shaderProgram;
		}

		ShaderProgram* getShaderProgram() {
			return shaderProgram;
		}
	protected:
		ShaderProgram* shaderProgram;
	};
}