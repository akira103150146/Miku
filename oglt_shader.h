#pragma once

#include "std_util.h"
#include "oglt_util.h"

#include <glm\glm.hpp>

namespace oglt {
	/********************************

	Class:		Shader

	Purpose:	Wraps OpenGL shader loading
	and compiling.

	********************************/

	class Shader
	{
	public:
		bool loadShader(string sFile, int a_iType);
		void deleteShader();

		bool getLinesFromFile(string sFile, bool bIncludePart, vector<string>* vResult);

		bool isLoaded();
		uint getShaderID();

		Shader();

	private:
		uint shaderId; // ID of shader
		int shaderType; // GL_VERTEX_SHADER, GL_FRAGMENT_SHADER...
		bool loaded; // Whether shader was loaded and compiled
	};

	/********************************

	Class:		ShaderProgram

	Purpose:	Wraps OpenGL shader program
	and make its usage easy.

	********************************/

	class ShaderProgram
	{
	public:
		void createProgram();
		void deleteProgram();

		bool addShaderToProgram(Shader* shShader);
		bool linkProgram();

		void useProgram();

		uint getProgramID();

		// Setting vectors
		void setUniform(string sName, glm::vec2* vVectors, int iCount = 1);
		void setUniform(string sName, const glm::vec2 vVector);
		void setUniform(string sName, glm::vec3* vVectors, int iCount = 1);
		void setUniform(string sName, const glm::vec3 vVector);
		void setUniform(string sName, glm::vec4* vVectors, int iCount = 1);
		void setUniform(string sName, const glm::vec4 vVector);

		// Setting floats
		void setUniform(string sName, float* fValues, int iCount = 1);
		void setUniform(string sName, const float fValue);

		// Setting 3x3 matrices
		void setUniform(string sName, glm::mat3* mMatrices, int iCount = 1);
		void setUniform(string sName, const glm::mat3 mMatrix);

		// Setting 4x4 matrices
		void setUniform(string sName, glm::mat4* mMatrices, int iCount = 1);
		void setUniform(string sName, const glm::mat4 mMatrix);

		// Setting integers
		void setUniform(string sName, int* iValues, int iCount = 1);
		void setUniform(string sName, const int iValue);

		// Model and normal matrix setting ispretty common
		void setModelAndNormalMatrix(string sModelMatrixName, string sNormalMatrixName, glm::mat4 mModelMatrix);
		void setModelAndNormalMatrix(string sModelMatrixName, string sNormalMatrixName, glm::mat4* mModelMatrix);

		ShaderProgram();

	private:
		uint programId; // ID of program
		bool linked; // Whether program was linked and is ready to use
	};
}