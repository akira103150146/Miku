#pragma once

#include "oglt_irenderable.h"
#include "oglt_resource.h"
#include "oglt_skeleton_material.h"
#include "oglt_mesh.h"
#include "oglt_vbo.h"

#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include <fbxsdk.h>

namespace oglt {
	class FbxModel : public IRenderable {
	public:
		FbxModel();
		~FbxModel();

		static bool initialize();
		static void destroyManager();
		bool load(const string& fileName);
		vector<Mesh>* getMeshs();
		void updateAnimation(float deltaTime);
		void setTimer(float timer);

		virtual void render(int renderType = OGLT_RENDER_SELF);
	private:
		struct VertexBoneData {
			uint boneIndices[4];
			float weights[4];

			VertexBoneData() {
				memset(boneIndices, 0, sizeof(boneIndices));
				memset(weights, 0, sizeof(weights));
			}

			void addBoneData(uint boneIndex, float wieght);
		};

		struct BoneInfo {
			glm::mat4 boneOffset;
			glm::mat4 finalTransform;
		};

		bool loadFromScene(FbxScene* scene);
		void processNode(FbxNode* node);
		void processMesh(FbxNode* node);
		void processSkeleton(FbxNode* node);
		void processLight(FbxNode* node);
		void processCamera(FbxNode* node);

		void readVertex(FbxMesh* mesh, int ctrlPointIndex, glm::vec3* outVertex);
		void readColor(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, glm::vec4* outColor);
		void readUV(FbxMesh* mesh, int ctrlPointIndex, int textureUVIndex, int uvLayer, glm::vec2* outUV);
		void readNormal(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, glm::vec3* outNormal);
		void readTangent(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, glm::vec3* outTangent);

		void connectMtlToMesh(FbxMesh* fbxMesh, Mesh* ogltMesh);
		void reconnectMtlToMesh(Mesh* mesh, vector<uint>& newMaterialIds);
		void loadMaterial(FbxMesh* mesh, vector<uint>& newMaterialIds);
		void loadMaterialAttribute(FbxSurfaceMaterial* surfaceMaterial, IMaterial* outMaterial);
		void loadMaterialTexture(FbxSurfaceMaterial* surfaceMaterial, IMaterial* outMaterial);
		void loadTexture(FbxTexture* texture, MaterialParam param, IMaterial* outMaterial);

		void connectSkeletonToMesh(FbxMesh* fbxMesh, vector<VertexBoneData>& ctrlPointBones);
		void connectSkinToMesh(FbxSkin* skin, vector<VertexBoneData>& ctrlPointBones);
		void mapVertexBoneFromCtrlPoint(vector<VertexBoneData>& ctrlPointBones, vector<int>& ctrlPointIndices);

		void finalizeVBO();

		glm::mat4 toGlmMatrix(FbxAMatrix& matrix);
		glm::mat4 toGlmMatrix(FbxMatrix& matrix);

		glm::vec3 toGlmVec3(FbxVector4& fbxVec4);
		glm::vec4 toGlmVec4(FbxVector4& fbxVec4);

		glm::quat toGlmQuat(FbxQuaternion& fbxQuat);

#define MAX_UV_CHANNEL 5

		bool loaded;
		vector<Mesh> meshs;

		uint vao;
		VertexBufferObject vertices;
		VertexBufferObject boneIndices;
		VertexBufferObject boneWeights;
		VertexBufferObject colors;
		VertexBufferObject normals;
		VertexBufferObject uvs[MAX_UV_CHANNEL];

		map<string, uint> boneMapping;
		vector<BoneInfo> boneInfos;
		vector<glm::mat4> boneTransforms;
		vector<FbxNode*> boneNodes;

		float timer;
		glm::mat4 globalTransform;

		static FbxManager* manager;
		FbxImporter* importer;
		FbxScene* scene;

		bool hasAnimation;
		string sPath;

		map<string, uint> animStackMap;
		map<string, uint> animLayerMap;
	};
}