#include "oglt_assimp_model.h"

#pragma comment(lib, "assimp.lib")

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

using namespace oglt;

AssimpModel::AssimpModel()
{
	loaded = false;
}

/*-----------------------------------------------

Name:	loadModelFromFile

Params:	sFilePath - guess ^^

Result: Loads model using Assimp library.

/*---------------------------------------------*/

bool AssimpModel::loadModelFromFile(char* sFilePath)
{
	vboData.createVBO();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(sFilePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene)
	{
		fprintf(stderr, "Couldn't load model %s", sFilePath);
		return false;
	}

	const int iVertexTotalSize = sizeof(aiVector3D) * 2 + sizeof(aiVector2D);

	int iTotalVertices = 0;

	FOR(i, scene->mNumMeshes)
	{
		aiMesh* mesh = scene->mMeshes[i];
		int iMeshFaces = mesh->mNumFaces;
		materialIndices.push_back(mesh->mMaterialIndex);
		int iSizeBefore = vboData.getCurrentSize();
		meshStartIndices.push_back(iSizeBefore / iVertexTotalSize);
		FOR(j, iMeshFaces)
		{
			const aiFace& face = mesh->mFaces[j];
			FOR(k, 3)
			{
				aiVector3D pos = mesh->mVertices[face.mIndices[k]];
				aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[k]];
				aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]] : aiVector3D(1.0f, 1.0f, 1.0f);
				vboData.addData(&pos, sizeof(aiVector3D));
				vboData.addData(&uv, sizeof(aiVector2D));
				vboData.addData(&normal, sizeof(aiVector3D));
			}
		}
		int iMeshVertices = mesh->mNumVertices;
		iTotalVertices += iMeshVertices;
		meshSizes.push_back((vboData.getCurrentSize() - iSizeBefore) / iVertexTotalSize);
	}
	numMaterials = scene->mNumMaterials;

	vector<int> materialRemap(numMaterials);

	FOR(i, numMaterials)
	{
		const aiMaterial* material = scene->mMaterials[i];
		int a = 5;
		int texIndex = 0;
		aiString path;  // filename

		if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &path) == AI_SUCCESS)
		{
			string sDir = getDirectoryPath(sFilePath);
			string sTextureName = path.data;
			string sFullPath = sDir + sTextureName;
			int iTexFound = -1;
			FOR(j, ESZ(textures))if (sFullPath == textures[j].getPath())
			{
				iTexFound = j;
				break;
			}
			if (iTexFound != -1)materialRemap[i] = iTexFound;
			else
			{
				Texture tNew;
				tNew.loadTexture2D(sFullPath);
				materialRemap[i] = ESZ(textures);
				textures.push_back(tNew);
			}
		}
	}

	FOR(i, ESZ(meshSizes))
	{
		int iOldIndex = materialIndices[i];
		materialIndices[i] = materialRemap[iOldIndex];
	}

	finalizeVBO();

	return loaded = true;
}

/*-----------------------------------------------

Name:	finalizeVBO

Params: none

Result: Uploads all loaded model data in one global
models' VBO.

/*---------------------------------------------*/

void AssimpModel::finalizeVBO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	vboData.bindVBO();
	vboData.uploadDataToGPU(GL_STATIC_DRAW);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), 0);
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)(sizeof(aiVector3D) + sizeof(aiVector2D)));
	// Texture coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)sizeof(aiVector3D));
}

/*-----------------------------------------------

Name:	render

Params: int render type

Result: Guess what it does ^^.

/*---------------------------------------------*/

void AssimpModel::render(int renderType)
{
	if (!loaded) return;

	if (shaderProgram != NULL) {
		shaderProgram->useProgram();
		shaderProgram->setModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mutexModelMatrix);
		shaderProgram->setUniform("matrices.viewMatrix", mutexViewMatrix);
		shaderProgram->setUniform("matrices.projMatrix", mutexProjMatrix);
		shaderProgram->setUniform("sunLight.vColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shaderProgram->setUniform("sunLight.vDirection", mutexSunLightDir);
		shaderProgram->setUniform("sunLight.fAmbient", 1.0f);
		shaderProgram->setUniform("gSampler", 0);
		shaderProgram->setUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	glBindVertexArray(vao);
	int iNumMeshes = ESZ(meshSizes);
	FOR(i, iNumMeshes)
	{
		int iMatIndex = materialIndices[i];
		textures[iMatIndex].bindTexture();
		glDrawArrays(GL_TRIANGLES, meshStartIndices[i], meshSizes[i]);
	}
}