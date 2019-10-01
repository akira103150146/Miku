#include "oglt_shader.h"

#include <glm/gtc/type_ptr.hpp>

using namespace oglt;

Shader::Shader()
{
	loaded = false;
}

/*-----------------------------------------------

Name:    loadShader

Params:  sFile - path to a file
a_iType - type of shader (fragment, vertex, geometry)

Result:	Loads and compiles shader.

/*---------------------------------------------*/

bool Shader::loadShader(string sFile, int a_iType)
{
	vector<string> sLines;

	if (!getLinesFromFile(sFile, false, &sLines)) return false;

	const char** sProgram = new const char*[ESZ(sLines)];
	FOR(i, ESZ(sLines))sProgram[i] = sLines[i].c_str();

	shaderId = glCreateShader(a_iType);

	glShaderSource(shaderId, ESZ(sLines), sProgram, NULL);
	glCompileShader(shaderId);

	delete[] sProgram;

	int iCompilationStatus;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &iCompilationStatus);

	if (iCompilationStatus == GL_FALSE)
	{
		char sInfoLog[1024];
		char sFinalMessage[1536];
		int iLogLength;
		glGetShaderInfoLog(shaderId, 1024, &iLogLength, sInfoLog);
		sprintf(sFinalMessage, "Error! Shader file %s wasn't compiled! The compiler returned:\n\n%s", sFile.c_str(), sInfoLog);
		cout << sFinalMessage << endl;
		return false;
	}
	shaderType = a_iType;
	loaded = true;

	return true;
}

/*-----------------------------------------------

Name:    getLinesFromFile

Params:  sFile - path to a file
bIncludePart - whether to add include part only
vResult - vector of strings to store result to

Result:  Loads and adds include part.

/*---------------------------------------------*/

bool Shader::getLinesFromFile(string sFile, bool bIncludePart, vector<string>* vResult)
{
	FILE* fp = fopen(sFile.c_str(), "rt");
	if (!fp)return false;

	string sDirectory;
	int slashIndex = -1;
	RFOR(i, ESZ(sFile) - 1)
	{
		if (sFile[i] == '\\' || sFile[i] == '/')
		{
			slashIndex = i;
			break;
		}
	}

	sDirectory = sFile.substr(0, slashIndex + 1);

	// Get all lines from a file

	char sLine[255];

	bool bInIncludePart = false;

	while (fgets(sLine, 255, fp))
	{
		stringstream ss(sLine);
		string sFirst;
		ss >> sFirst;
		if (sFirst == "#include")
		{
			string sFileName;
			ss >> sFileName;
			if (ESZ(sFileName) > 0 && sFileName[0] == '\"' && sFileName[ESZ(sFileName) - 1] == '\"')
			{
				sFileName = sFileName.substr(1, ESZ(sFileName) - 2);
				getLinesFromFile(sDirectory + sFileName, true, vResult);
			}
		}
		else if (sFirst == "#include_part")
			bInIncludePart = true;
		else if (sFirst == "#definition_part")
			bInIncludePart = false;
		else if (!bIncludePart || (bIncludePart && bInIncludePart))
			vResult->push_back(sLine);
	}
	fclose(fp);

	return true;
}

/*-----------------------------------------------

Name:	isLoaded

Params:	none

Result:	True if shader was loaded and compiled.

/*---------------------------------------------*/

bool Shader::isLoaded()
{
	return loaded;
}

/*-----------------------------------------------

Name:	GetShaderID

Params:	none

Result:	Returns ID of a generated shader.

/*---------------------------------------------*/

uint Shader::getShaderID()
{
	return shaderId;
}

/*-----------------------------------------------

Name:	DeleteShader

Params:	none

Result:	Deletes shader and frees memory in GPU.

/*---------------------------------------------*/

void Shader::deleteShader()
{
	if (!isLoaded())return;
	loaded = false;
	glDeleteShader(shaderId);
}

ShaderProgram::ShaderProgram()
{
	linked = false;
}

/*-----------------------------------------------

Name:	CreateProgram

Params:	none

Result:	Creates a new program.

/*---------------------------------------------*/

void ShaderProgram::createProgram()
{
	programId = glCreateProgram();
}

/*-----------------------------------------------

Name:	AddShaderToProgram

Params:	sShader - shader to add

Result:	Adds a shader (like source file) to
a program, but only compiled one.

/*---------------------------------------------*/

bool ShaderProgram::addShaderToProgram(Shader* shShader)
{
	if (!shShader->isLoaded())return false;

	glAttachShader(programId, shShader->getShaderID());

	return true;
}

/*-----------------------------------------------

Name:	LinkProgram

Params:	none

Result:	Performs final linkage of OpenGL program.

/*---------------------------------------------*/

bool ShaderProgram::linkProgram()
{
	glLinkProgram(programId);
	int iLinkStatus;
	glGetProgramiv(programId, GL_LINK_STATUS, &iLinkStatus);
	linked = iLinkStatus == GL_TRUE;
	return linked;
}

/*-----------------------------------------------

Name:	DeleteProgram

Params:	none

Result:	Deletes program and frees memory on GPU.

/*---------------------------------------------*/

void ShaderProgram::deleteProgram()
{
	if (!linked)return;
	linked = false;
	glDeleteProgram(programId);
}

/*-----------------------------------------------

Name:	UseProgram

Params:	none

Result:	Tells OpenGL to use this program.

/*---------------------------------------------*/

void ShaderProgram::useProgram()
{
	if (linked)glUseProgram(programId);
}

/*-----------------------------------------------

Name:	GetProgramID

Params:	none

Result:	Returns OpenGL generated shader program ID.

/*---------------------------------------------*/

uint ShaderProgram::getProgramID()
{
	return programId;
}

/*-----------------------------------------------

Name:	UniformSetters

Params:	yes, there are :)

Result:	These set of functions set most common
types of uniform variables.

/*---------------------------------------------*/

// Setting floats

void ShaderProgram::setUniform(string sName, float* fValues, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform1fv(iLoc, iCount, fValues);
}

void ShaderProgram::setUniform(string sName, const float fValue)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform1fv(iLoc, 1, &fValue);
}

// Setting vectors

void ShaderProgram::setUniform(string sName, glm::vec2* vVectors, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform2fv(iLoc, iCount, (GLfloat*)vVectors);
}

void ShaderProgram::setUniform(string sName, const glm::vec2 vVector)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform2fv(iLoc, 1, (GLfloat*)&vVector);
}

void ShaderProgram::setUniform(string sName, glm::vec3* vVectors, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform3fv(iLoc, iCount, (GLfloat*)vVectors);
}

void ShaderProgram::setUniform(string sName, const glm::vec3 vVector)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform3fv(iLoc, 1, (GLfloat*)&vVector);
}

void ShaderProgram::setUniform(string sName, glm::vec4* vVectors, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform4fv(iLoc, iCount, (GLfloat*)vVectors);
}

void ShaderProgram::setUniform(string sName, const glm::vec4 vVector)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform4fv(iLoc, 1, (GLfloat*)&vVector);
}

// Setting 3x3 matrices

void ShaderProgram::setUniform(string sName, glm::mat3* mMatrices, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniformMatrix3fv(iLoc, iCount, GL_FALSE, (GLfloat*)mMatrices);
}

void ShaderProgram::setUniform(string sName, const glm::mat3 mMatrix)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniformMatrix3fv(iLoc, 1, GL_FALSE, (GLfloat*)&mMatrix);
}

// Setting 4x4 matrices

void ShaderProgram::setUniform(string sName, glm::mat4* mMatrices, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniformMatrix4fv(iLoc, iCount, GL_FALSE, (GLfloat*)mMatrices);
}

void ShaderProgram::setUniform(string sName, const glm::mat4 mMatrix)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniformMatrix4fv(iLoc, 1, GL_FALSE, (GLfloat*)&mMatrix);
}

// Setting integers

void ShaderProgram::setUniform(string sName, int* iValues, int iCount)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform1iv(iLoc, iCount, iValues);
}

void ShaderProgram::setUniform(string sName, const int iValue)
{
	int iLoc = glGetUniformLocation(programId, sName.c_str());
	glUniform1i(iLoc, iValue);
}

void ShaderProgram::setModelAndNormalMatrix(string sModelMatrixName, string sNormalMatrixName, glm::mat4 mModelMatrix)
{
	setUniform(sModelMatrixName, mModelMatrix);
	setUniform(sNormalMatrixName, glm::transpose(glm::inverse(mModelMatrix)));
}

void ShaderProgram::setModelAndNormalMatrix(string sModelMatrixName, string sNormalMatrixName, glm::mat4* mModelMatrix)
{
	setUniform(sModelMatrixName, mModelMatrix);
	setUniform(sNormalMatrixName, glm::transpose(glm::inverse(*mModelMatrix)));
}