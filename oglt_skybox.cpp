#include "oglt_skybox.h"

using namespace oglt;
using namespace oglt::scene;
/*-----------------------------------------------

Name:	load

Params:	paths of skybox pictures

Result:	Loads skybox and creates VAO and VBO for it.

/*---------------------------------------------*/

void Skybox::load(string a_sDirectory, string a_sFront, string a_sBack, string a_sLeft, string a_sRight, string a_sTop, string a_sBottom)
{
	textures[0].loadTexture2D(a_sDirectory + a_sFront);
	textures[1].loadTexture2D(a_sDirectory + a_sBack);
	textures[2].loadTexture2D(a_sDirectory + a_sLeft);
	textures[3].loadTexture2D(a_sDirectory + a_sRight);
	textures[4].loadTexture2D(a_sDirectory + a_sTop);
	textures[5].loadTexture2D(a_sDirectory + a_sBottom);

	sDirectory = a_sDirectory;

	sFront = a_sFront;
	sBack = a_sBack;
	sLeft = a_sLeft;
	sRight = a_sRight;
	sTop = a_sTop;
	sBottom = a_sBottom;

	FOR(i, 6)
	{
		textures[i].setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR);
		textures[i].setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		textures[i].setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vboData.createVBO();
	vboData.bindVBO();

	glm::vec3 vSkyBoxVertices[24] =
	{
		// Front face
		glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(200.0f, -200.0f, 200.0f), glm::vec3(-200.0f, 200.0f, 200.0f), glm::vec3(-200.0f, -200.0f, 200.0f),
		// Back face
		glm::vec3(-200.0f, 200.0f, -200.0f), glm::vec3(-200.0f, -200.0f, -200.0f), glm::vec3(200.0f, 200.0f, -200.0f), glm::vec3(200.0f, -200.0f, -200.0f),
		// Left face
		glm::vec3(-200.0f, 200.0f, 200.0f), glm::vec3(-200.0f, -200.0f, 200.0f), glm::vec3(-200.0f, 200.0f, -200.0f), glm::vec3(-200.0f, -200.0f, -200.0f),
		// Right face
		glm::vec3(200.0f, 200.0f, -200.0f), glm::vec3(200.0f, -200.0f, -200.0f), glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(200.0f, -200.0f, 200.0f),
		// Top face
		glm::vec3(-200.0f, 200.0f, -200.0f), glm::vec3(200.0f, 200.0f, -200.0f), glm::vec3(-200.0f, 200.0f, 200.0f), glm::vec3(200.0f, 200.0f, 200.0f),
		// Bottom face
		glm::vec3(200.0f, -200.0f, -200.0f), glm::vec3(-200.0f, -200.0f, -200.0f), glm::vec3(200.0f, -200.0f, 200.0f), glm::vec3(-200.0f, -200.0f, 200.0f),
	};
	glm::vec2 vSkyBoxTexCoords[4] =
	{
		glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)
	};

	glm::vec3 vSkyBoxNormals[6] =
	{
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	};

	FOR(i, 24)
	{
		vboData.addData(&vSkyBoxVertices[i], sizeof(glm::vec3));
		vboData.addData(&vSkyBoxTexCoords[i % 4], sizeof(glm::vec2));
		vboData.addData(&vSkyBoxNormals[i / 4], sizeof(glm::vec3));
	}

	vboData.uploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
	// Texture coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)sizeof(glm::vec3));
}

/*-----------------------------------------------

Name:	RenderSkybox

Params:	none

Result: Guess what it does :)

/*---------------------------------------------*/

void Skybox::render(int renderType)
{
	if (shaderProgram != NULL) {
		shaderProgram->useProgram();
		shaderProgram->setModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", modelMatrix);
		shaderProgram->setUniform("matrices.viewMatrix", mutexViewMatrix);
		shaderProgram->setUniform("matrices.projMatrix", mutexProjMatrix);
		shaderProgram->setUniform("sunLight.vColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shaderProgram->setUniform("sunLight.vDirection", mutexSunLightDir);
		shaderProgram->setUniform("sunLight.fAmbient", 1.0f);
		shaderProgram->setUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shaderProgram->setUniform("gSampler", 0);
	}

	glDepthMask(0);
	glDisable(GL_DEPTH);
	glBindVertexArray(vao);
	FOR(i, 6)
	{
		textures[i].bindTexture();
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}
	glEnable(GL_DEPTH);
	glDepthMask(1);
}

bool Skybox::operator==(const Skybox& other)
{
	return this->sDirectory == other.sDirectory
		&& this->sFront == other.sFront
		&& this->sBack == other.sBack
		&& this->sLeft == other.sLeft
		&& this->sRight == other.sRight
		&& this->sTop == other.sTop
		&& this->sBottom == other.sBottom;
}

/*-----------------------------------------------

Name:	DeleteSkybox

Params:	none

Result:	Deletes skybox and all associated memory.

/*---------------------------------------------*/

void Skybox::deleteSkybox()
{
	FOR(i, 6)textures[i].deleteTexture();
	glDeleteVertexArrays(1, &vao);
	vboData.deleteVBO();
}