#include "oglt_fbx_model.h"

#include <fbxsdk\fileio\fbxiosettings.h>
#include <glm\gtc\matrix_transform.hpp>

#include "oglt_skeleton_material.h"

using namespace oglt;
using namespace glm;

FbxManager* FbxModel::manager;

FbxModel::FbxModel()
{
	loaded = false;
	timer = 0.0f;
	hasAnimation = false;
}

FbxModel::~FbxModel()
{

}

bool FbxModel::initialize()
{
	manager = FbxManager::Create();
	if (manager == NULL) {
		fprintf(stderr, "Error: Unable to create Fbx Manager!!!\n");
		return false;
	}
	else {
		cout << "Autodesk FBX SDK version " << manager->GetVersion() << endl;
	}

	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString path = FbxGetApplicationDirectory();
	manager->LoadPluginsDirectory(path.Buffer());
	
	return true;
}

void FbxModel::destroyManager()
{
	manager->Destroy();
}

vector<Mesh>* FbxModel::getMeshs()
{
	return &meshs;
}

void FbxModel::setTimer(float timer)
{
	this->timer = timer;
}

bool FbxModel::load(const string & fileName)
{
	if (manager == NULL) {
		fprintf(stderr, "Error: The Fbx Manager is not initialze.\n");
		return false;
	}

	// Create the importer.
	int fileFormat = -1;
	importer = FbxImporter::Create(manager, "My Importer");
	if (manager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName.c_str(), fileFormat))
	{
		// Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
		fileFormat = manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}

	// Initialize the importer by providing a filename.
	if (importer->Initialize(fileName.c_str(), fileFormat) == true)
	{
		cout << "Importing file " << fileName << endl;
		cout << "Please wait!" << endl;
	}
	else
	{
		fprintf(stderr, "Unable to open file %s \n Error reported: %s\n",
			fileName, importer->GetStatus().GetErrorString());
		return false;
	}

	scene = FbxScene::Create(manager, "My Scene");
	if (scene == NULL)
	{
		fprintf(stderr, "Error: Unable to create FBX scene!\n");
		return false;
	}

	if (!importer->Import(scene)) {
		fprintf(stderr, "Error: Unable importer to scene.\n");
		return false;
	}

	FbxSystemUnit sceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
	if (sceneSystemUnit.GetScaleFactor() != 1.0)
	{
		//The unit in this is centimeter.
		FbxSystemUnit::cm.ConvertScene(scene);
	}

	FbxGeometryConverter geoConverter(manager);
	
	if (!geoConverter.Triangulate(scene, true)) {
		fprintf(stderr, "Error: Unable conver to triangle mesh.\n");
	}

	importer->Destroy();
	importer = NULL;

	sPath = fileName;

	loadFromScene(scene);

	loaded = true;

	cout << "vertices size: " << vertices.getCurrentSize() / sizeof(glm::vec3) << endl;
	cout << "colors size: " << colors.getCurrentSize() / sizeof(glm::vec4) << endl;
	cout << "uv0s size: " << uvs[0].getCurrentSize() / sizeof(glm::vec2) << endl;
	cout << "normals size: " << normals.getCurrentSize() / sizeof(glm::vec3) << endl;
	cout << "vertex bone indices size: " << boneIndices.getCurrentSize() / (4 * sizeof(int)) << endl;
	cout << "vetex bone weights size: " << boneWeights.getCurrentSize() / (4 * sizeof(float)) << endl;
	cout << "bone infos size: " << boneInfos.size() << endl;
	cout << "mesh size: " << meshs.size() << endl;
	FOR(i, meshs.size()) {
		cout << "attribute on mesh " << i << endl;
		cout << "start index: " << meshs[i].startIndex << endl;
		cout << "size: " << meshs[i].size << endl;
		char buf[100];
		switch (meshs[i].mtlMapMode) {
		case NONE:
			sprintf(buf, "NONE");
			break;
		case ALL_SAME:
			sprintf(buf, "All Same");
			break;
		case BY_POLYGON:
			sprintf(buf, "By Polygon");
			break;
		}
		cout << "material mapping mode: " << buf << endl;
		if (meshs[i].mtlMapMode == BY_POLYGON) {
			cout << "the polygons size: " << meshs[i].polygons.size() << endl;
			FOR(j, ESZ(meshs[i].polygons)) {
				cout << "polygon " << j << " start index: " << meshs[i].polygons[j].startIndex << endl;
				cout << "polygon " << j << " size: " << meshs[i].polygons[j].size << endl;
				cout << "polygon " << j << " material index: " << meshs[i].polygons[j].materialId << endl;
				IMaterial* material = Resource::instance()->getMaterial(meshs[i].polygons[j].materialId);
				if (material != NULL) {
					cout << "polygon " << j << " material name: " << material->getName() << endl;
				}
			}
		}
		cout << endl;
	}

	finalizeVBO();

	return true;
}

bool FbxModel::loadFromScene(FbxScene * scene)
{
	FbxNode* rootNode = scene->GetRootNode();

	if (rootNode == NULL) {
		return false;
	}

	vertices.createVBO();
	colors.createVBO();
	normals.createVBO();
	FOR(i, MAX_UV_CHANNEL) {
		uvs[i].createVBO();
	}
	boneIndices.createVBO();
	boneWeights.createVBO();

	hasAnimation = scene->GetSrcObjectCount<FbxAnimStack>() == 0 ? false : true;
	FOR(i, scene->GetSrcObjectCount<FbxAnimStack>()) {
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
		string stackName = animStack->GetName();
		animStackMap[stackName] = i;
		FOR(j, scene->GetSrcObjectCount<FbxAnimLayer>()) {
			FbxAnimLayer* animLayer = scene->GetSrcObject<FbxAnimLayer>(j);
			string layerName = animLayer->GetName();
			animLayerMap[layerName] = j;
		}
	}

	cout << "Processing file " + sPath << endl;
	cout << "Please wait!" << endl;
	processNode(rootNode);

	boneTransforms.resize(boneInfos.size());
}

void FbxModel::processNode(FbxNode * node)
{
	if (node->GetNodeAttribute()) {
		switch (node->GetNodeAttribute()->GetAttributeType()) {
		case FbxNodeAttribute::eMesh:
			processMesh(node);
			break;
		case FbxNodeAttribute::eSkeleton:
			break;
		case FbxNodeAttribute::eLight:
			break;
		case FbxNodeAttribute::eCamera:
			break;
		}
	}

	FOR(i, node->GetChildCount()) {
		processNode(node->GetChild(i));
	}
}

void FbxModel::processMesh(FbxNode * node)
{
	FbxMesh* mesh = node->GetMesh();
	if (mesh == NULL) {
		return;
	}

	glm::vec3 vertex[3];
	glm::vec4 color[3];
	glm::vec3 normal[3];
	glm::vec3 tangent[3];
	glm::vec2 uv[3][MAX_UV_CHANNEL];
	Triangle triangle;

	vector<int> ctrlPointIndices;
	vector<VertexBoneData> ctrlPointBones;

	Mesh meshEntry;
	meshEntry.startIndex = vertices.getCurrentSize() / sizeof(glm::vec3);

	int vertexCounter = 0;

	FOR(i, mesh->GetPolygonCount()) {
		triangle.startIndex = vertices.getCurrentSize() / sizeof(glm::vec3);
		FOR (j, 3) {
			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
			ctrlPointIndices.push_back(ctrlPointIndex);

			readVertex(mesh, ctrlPointIndex, &vertex[j]);

			readColor(mesh, ctrlPointIndex, vertexCounter, &color[j]);

			FOR(k, mesh->GetTextureUVCount()) {
				if (k >= MAX_UV_CHANNEL)
					break;
				readUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), k, &(uv[j][k]));
				uvs[k].addData(&uv[j][k], sizeof(glm::vec2));
			}

			readNormal(mesh, ctrlPointIndex, vertexCounter, &normal[j]);

			readTangent(mesh, ctrlPointIndex, vertexCounter, &tangent[j]);

			vertexCounter++;

			triangle.vertexIndices[j] = vertices.getCurrentSize() / sizeof(glm::vec3);
			vertices.addData(&vertex[j], sizeof(glm::vec3));
			colors.addData(&color[j], sizeof(glm::vec4));
			normals.addData(&normal[j], sizeof(glm::vec3));
		}
		meshEntry.triangles.push_back(triangle);
	}

	if (hasAnimation) {
		connectSkeletonToMesh(mesh, ctrlPointBones);
		mapVertexBoneFromCtrlPoint(ctrlPointBones, ctrlPointIndices);
	}

	vector<uint> newMaterialIds;
	connectMtlToMesh(mesh, &meshEntry);
	loadMaterial(mesh, newMaterialIds);
	reconnectMtlToMesh(&meshEntry, newMaterialIds);

	if (meshEntry.mtlMapMode != BY_POLYGON) {
		// if the materials are not use the polygon mapping
		// clear the triangles array for reduce memory cost
		meshEntry.triangles.clear();
	}
	else {
		// if use polygon mapping build the polygons for rendering
		meshEntry.buildPolygonsPerMtl();
	}

	meshEntry.size = vertices.getCurrentSize() / sizeof(glm::vec3) - meshEntry.startIndex;
	meshs.push_back(meshEntry);
}

void FbxModel::processSkeleton(FbxNode * node)
{
}

void FbxModel::processLight(FbxNode * node)
{
}

void FbxModel::processCamera(FbxNode * node)
{
}

void FbxModel::readVertex(FbxMesh * mesh, int ctrlPointIndex, glm::vec3 * outVertex)
{
	FbxVector4 ctrlPoint = mesh->GetControlPointAt(ctrlPointIndex);
	outVertex->x = ctrlPoint[0];
	outVertex->y = ctrlPoint[1];
	outVertex->z = ctrlPoint[2];
}

void FbxModel::readColor(FbxMesh * mesh, int ctrlPointIndex, int vertexCounter, glm::vec4 * outColor)
{
	if (mesh->GetElementVertexColorCount() < 1) {
		return;
	}

	FbxGeometryElementVertexColor* vertexColor = mesh->GetElementVertexColor(0);
	if (vertexColor == NULL) {
		outColor->x = 1.0f;
		outColor->y = 1.0f;
		outColor->z = 1.0f;
		outColor->w = 1.0f;
		return;
	}

	switch (vertexColor->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint:
		switch (vertexColor->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outColor->x = vertexColor->GetDirectArray().GetAt(ctrlPointIndex).mRed;
			outColor->y = vertexColor->GetDirectArray().GetAt(ctrlPointIndex).mGreen;
			outColor->z = vertexColor->GetDirectArray().GetAt(ctrlPointIndex).mBlue;
			outColor->w = vertexColor->GetDirectArray().GetAt(ctrlPointIndex).mAlpha;
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = vertexColor->GetIndexArray().GetAt(ctrlPointIndex);
			outColor->x = vertexColor->GetDirectArray().GetAt(id).mRed;
			outColor->y = vertexColor->GetDirectArray().GetAt(id).mGreen;
			outColor->z = vertexColor->GetDirectArray().GetAt(id).mBlue;
			outColor->w = vertexColor->GetDirectArray().GetAt(id).mAlpha;
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexColor->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outColor->x = vertexColor->GetDirectArray().GetAt(vertexCounter).mRed;
			outColor->y = vertexColor->GetDirectArray().GetAt(vertexCounter).mGreen;
			outColor->z = vertexColor->GetDirectArray().GetAt(vertexCounter).mBlue;
			outColor->w = vertexColor->GetDirectArray().GetAt(vertexCounter).mAlpha;
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = vertexColor->GetIndexArray().GetAt(vertexCounter);
			outColor->x = vertexColor->GetDirectArray().GetAt(id).mRed;
			outColor->y = vertexColor->GetDirectArray().GetAt(id).mGreen;
			outColor->z = vertexColor->GetDirectArray().GetAt(id).mBlue;
			outColor->w = vertexColor->GetDirectArray().GetAt(id).mAlpha;
			break;
		}
		break;
	}
	
}

void FbxModel::readUV(FbxMesh * mesh, int ctrlPointIndex, int textureUVIndex, int uvLayer, glm::vec2 * outUV)
{
	if (uvLayer >= mesh->GetTextureUVCount())
		return;

	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(uvLayer);
	if (vertexUV == NULL) {
		return;
	}

	switch (vertexUV->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outUV->x = vertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
			outUV->x = vertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = vertexUV->GetIndexArray().GetAt(ctrlPointIndex);
			outUV->x = vertexUV->GetDirectArray().GetAt(id)[0];
			outUV->x = vertexUV->GetDirectArray().GetAt(id)[1];
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
			outUV->x = vertexUV->GetDirectArray().GetAt(textureUVIndex)[0];
			outUV->y = vertexUV->GetDirectArray().GetAt(textureUVIndex)[1];
			break;
		}
		break;
	}
}

void FbxModel::readNormal(FbxMesh * mesh, int ctrlPointIndex, int vertexCounter, glm::vec3 * outNormal)
{
	if (mesh->GetElementNormalCount() < 1)
		return;

	FbxGeometryElementNormal* normal = mesh->GetElementNormal(0);
	if (normal == NULL)
		return;
	
	switch (normal->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint:
		switch (normal->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outNormal->x = normal->GetDirectArray().GetAt(ctrlPointIndex)[0];
			outNormal->y = normal->GetDirectArray().GetAt(ctrlPointIndex)[1];
			outNormal->z = normal->GetDirectArray().GetAt(ctrlPointIndex)[2];
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = normal->GetIndexArray().GetAt(ctrlPointIndex);
			outNormal->x = normal->GetDirectArray().GetAt(id)[0];
			outNormal->y = normal->GetDirectArray().GetAt(id)[1];
			outNormal->z = normal->GetDirectArray().GetAt(id)[2];
			break;
		}

	case FbxGeometryElement::eByPolygonVertex:
		switch (normal->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outNormal->x = normal->GetDirectArray().GetAt(vertexCounter)[0];
			outNormal->y = normal->GetDirectArray().GetAt(vertexCounter)[1];
			outNormal->z = normal->GetDirectArray().GetAt(vertexCounter)[2];
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = normal->GetIndexArray().GetAt(vertexCounter);
			outNormal->x = normal->GetDirectArray().GetAt(id)[0];
			outNormal->y = normal->GetDirectArray().GetAt(id)[1];
			outNormal->z = normal->GetDirectArray().GetAt(id)[2];
			break;
		}
		break;
	}
}

void FbxModel::readTangent(FbxMesh * mesh, int ctrlPointIndex, int vertexCounter, glm::vec3 * outTangent)
{
	if (mesh->GetElementTangentCount() < 1)
		return;

	FbxGeometryElementTangent* tangent = mesh->GetElementTangent(0);
	if (tangent == NULL)
		return;

	switch (tangent->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint:
		switch (tangent->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outTangent->x = tangent->GetDirectArray().GetAt(ctrlPointIndex)[0];
			outTangent->y = tangent->GetDirectArray().GetAt(ctrlPointIndex)[1];
			outTangent->z = tangent->GetDirectArray().GetAt(ctrlPointIndex)[2];
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = tangent->GetIndexArray().GetAt(ctrlPointIndex);
			outTangent->x = tangent->GetDirectArray().GetAt(id)[0];
			outTangent->y = tangent->GetDirectArray().GetAt(id)[1];
			outTangent->z = tangent->GetDirectArray().GetAt(id)[2];
			break;
		}
	case FbxGeometryElement::eByPolygonVertex:
		switch (tangent->GetReferenceMode()) {
		case FbxGeometryElement::eDirect:
			outTangent->x = tangent->GetDirectArray().GetAt(vertexCounter)[0];
			outTangent->y = tangent->GetDirectArray().GetAt(vertexCounter)[1];
			outTangent->z = tangent->GetDirectArray().GetAt(vertexCounter)[2];
			break;
		case FbxGeometryElement::eIndexToDirect:
			int id = tangent->GetIndexArray().GetAt(vertexCounter);
			outTangent->x = tangent->GetDirectArray().GetAt(id)[0];
			outTangent->y = tangent->GetDirectArray().GetAt(id)[1];
			outTangent->z = tangent->GetDirectArray().GetAt(id)[2];
			break;
		}
		break;
	}
}

void FbxModel::connectMtlToMesh(FbxMesh * fbxMesh, Mesh * ogltMesh)
{
	FbxGeometryElementMaterial* material = fbxMesh->GetElementMaterial();
	if (material) {
		FbxLayerElementArrayTemplate<int>* mtlIndices = &material->GetIndexArray();
		if (mtlIndices) {
			switch (material->GetMappingMode()) {
			case FbxGeometryElement::eByPolygon:
				if (mtlIndices->GetCount() == ogltMesh->triangles.size()) {
					FOR(i, mtlIndices->GetCount()) {
						ogltMesh->triangles[i].materialId = mtlIndices->GetAt(i);
					}
					ogltMesh->mtlMapMode = BY_POLYGON;
				}
				break;
			case FbxGeometryElement::eAllSame:
				ogltMesh->materialId = mtlIndices->GetAt(0);
				ogltMesh->mtlMapMode = ALL_SAME;
				break;
			}
		}
	}
}

void FbxModel::reconnectMtlToMesh(Mesh * mesh, vector<oglt::uint>& newMaterialIds)
{
	if (mesh->mtlMapMode == BY_POLYGON) {
		FOR(i, ESZ(mesh->triangles)) {
			uint oldId = mesh->triangles[i].materialId;
			mesh->triangles[i].materialId = newMaterialIds[oldId];
		}
	}
	else if (mesh->mtlMapMode == ALL_SAME) {
		if (newMaterialIds.size() > 0) {
			mesh->materialId = newMaterialIds[0];
		}
	}
}

void FbxModel::loadMaterial(FbxMesh* mesh, vector<oglt::uint>& newMaterialIds)
{
	FbxNode* node = mesh->GetNode();
	if (node == NULL)
		return;

	uint materialCount = node->GetMaterialCount();

	if (materialCount > 0) {
		FOR(i, materialCount) {
			FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);
			SkeletonMaterial* material = new SkeletonMaterial();
			loadMaterialAttribute(surfaceMaterial, material);
			uint id = Resource::instance()->addMaterial(material);
			newMaterialIds.push_back(id);
		}
	}
}

void FbxModel::loadMaterialAttribute(FbxSurfaceMaterial * surfaceMaterial, IMaterial* outMaterial)
{
	outMaterial->setName(sPath + " mat: " + surfaceMaterial->GetName());
	if (surfaceMaterial->GetClassId().Is(FbxSurfacePhong::ClassId)) {
		FbxSurfacePhong* surfacePhone = (FbxSurfacePhong*)surfaceMaterial;
		FbxDouble3 fbxColor = surfacePhone->Ambient;
		vec3 color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(AMBIENT, color);
		fbxColor = surfacePhone->Diffuse;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(DIFFUSE, color);
		fbxColor = surfacePhone->Specular;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(SPECULAR, color);
		fbxColor = surfacePhone->Emissive;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(EMISSIVE, color);
		
		FbxDouble factor = surfacePhone->TransparencyFactor;
		outMaterial->setFactor(TRANSPARENCY_FACTOR, factor);
		factor = surfacePhone->Shininess;
		outMaterial->setFactor(SHININESS_FACTOR, factor);
		factor = surfacePhone->ReflectionFactor;
		outMaterial->setFactor(REFLECTION_FACTOR, factor);
	}
	else if (surfaceMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId)) {
		FbxSurfacePhong* surfacePhone = (FbxSurfacePhong*)surfaceMaterial;
		FbxDouble3 fbxColor = surfacePhone->Ambient;
		vec3 color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(AMBIENT, color);
		fbxColor = surfacePhone->Diffuse;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(DIFFUSE, color);
		fbxColor = surfacePhone->Specular;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(SPECULAR, color);
		fbxColor = surfacePhone->Emissive;
		color = vec3(fbxColor[0], fbxColor[1], fbxColor[2]);
		outMaterial->setColor(EMISSIVE, color);

		FbxDouble factor = surfacePhone->TransparencyFactor;
		outMaterial->setFactor(TRANSPARENCY_FACTOR, factor);
	}

	loadMaterialTexture(surfaceMaterial, outMaterial);
}

void FbxModel::loadMaterialTexture(FbxSurfaceMaterial * surfaceMaterial, IMaterial * outMaterial)
{
	FbxProperty property;
	property = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
	FOR(i, property.GetSrcObjectCount<FbxTexture>()) {
		FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
		if (layeredTexture) {
			FOR(j, layeredTexture->GetSrcObjectCount<FbxTexture>()) {
				FbxTexture* texture = layeredTexture->GetSrcObject<FbxTexture>(j);
				if (texture) {
					FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
					loadTexture(texture, DIFFUSE, outMaterial);
				}
			}
		}
		else {
			// no layered textures simply get on the property
			FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
			loadTexture(texture, DIFFUSE, outMaterial);
		}
	}
}

void FbxModel::loadTexture(FbxTexture * texture, MaterialParam param, IMaterial * outMaterial)
{
	if (texture) {
		FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
		if (fileTexture == NULL) return;
		uint textureId = Resource::instance()->addTexture(fileTexture->GetFileName());
		outMaterial->linkTexture(param, textureId);
	}
}

void FbxModel::connectSkeletonToMesh(FbxMesh * fbxMesh, vector<VertexBoneData>& ctrlPointBones)
{
	FbxDeformer* deformer;
	FbxSkin* skin;

	ctrlPointBones.resize(fbxMesh->GetControlPointsCount());
	FOR(i, fbxMesh->GetDeformerCount()) {
		deformer = fbxMesh->GetDeformer(i);
		if (deformer == NULL)
			continue;

		if (deformer->GetDeformerType() != FbxSkin::eSkin)
			continue;

		skin = FbxCast<FbxSkin>(deformer);
		if (skin == NULL)
			continue;

		connectSkinToMesh(skin, ctrlPointBones);
	}
}

void FbxModel::connectSkinToMesh(FbxSkin* skin, vector<VertexBoneData>& ctrlPointBones)
{
	FbxCluster* cluster;
	FbxNode* node;
	FbxAMatrix transformMatrix, linkMatrix;
	
	FOR(i, skin->GetClusterCount()) {
		cluster = skin->GetCluster(i);
		if (cluster == NULL)
			continue;

		node = cluster->GetLink();

		if (node == NULL)
			continue;

		cluster->GetTransformMatrix(transformMatrix);
		cluster->GetTransformLinkMatrix(linkMatrix);
		glm::mat4 glmLinkMatrix = toGlmMatrix(linkMatrix);;
		glm::mat4 glmTransformMatrix = toGlmMatrix(transformMatrix);

		string boneName = node->GetName();
		uint boneIndex;
		if (boneMapping.find(boneName) == boneMapping.end()) {
			BoneInfo boneInfo;
			boneInfo.boneOffset = glm::inverse(glmLinkMatrix) * glmTransformMatrix;
			boneInfo.finalTransform = glmLinkMatrix * boneInfo.boneOffset;
			boneTransforms.push_back(boneInfo.finalTransform);
			boneIndex = boneInfos.size();
			boneInfos.push_back(boneInfo);
			boneNodes.push_back(node);
			boneMapping[boneName] = boneIndex;
		}
		else {
			boneIndex = boneMapping[boneName];
		}

		int* ctrlPointIndices = cluster->GetControlPointIndices();
		double* ctrlPointWeights = cluster->GetControlPointWeights();

		FOR(i, cluster->GetControlPointIndicesCount()) {
			int ctrlPointIndex = ctrlPointIndices[i];
			float weight = ctrlPointWeights[i];
			ctrlPointBones[ctrlPointIndex].addBoneData(boneIndex, weight);
		}
	}
}

void FbxModel::mapVertexBoneFromCtrlPoint(vector<VertexBoneData>& ctrlPointBones, vector<int>& ctrlPointIndices)
{
	FOR(i, ESZ(ctrlPointIndices)) {
		VertexBoneData* vbd = &ctrlPointBones[ctrlPointIndices[i]];
		FOR(j, 4) {
			assert(vbd->boneIndices[j] < ESZ(boneInfos) && vbd->boneIndices[j] >= 0);
			assert(vbd->weights[j] >= 0.0f && vbd->weights[j] <= 1.0f);
		}
		boneIndices.addData(&vbd->boneIndices[0], sizeof(vbd->boneIndices));
		boneWeights.addData(&vbd->weights[0], sizeof(vbd->weights));
	}
}

void FbxModel::updateAnimation(float deltaTime)
{
	if (!hasAnimation)
		return;

	timer += deltaTime;
	
	FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(0);
	if (animStack == NULL) return;
	FbxAnimLayer* animLayer = animStack->GetSrcObject<FbxAnimLayer>();
	if (animLayer == NULL) return;

	FbxAnimEvaluator* animEvaluator = scene->GetAnimationEvaluator();
	if (animEvaluator == NULL) return;

	FbxTime time;
	time.SetSecondDouble(timer);

	vector<mat4> localBoneTransform;
	FOR(i, ESZ(boneInfos)) {
		FbxNode* node = boneNodes[i];
		FbxAMatrix localMatrix = animEvaluator->GetNodeLocalTransform(node, time);
		FbxAMatrix globalMatrix = animEvaluator->GetNodeGlobalTransform(node, time);
		
		boneInfos[i].finalTransform = toGlmMatrix(globalMatrix) * boneInfos[i].boneOffset;
		localBoneTransform.push_back(boneInfos[i].finalTransform);
	}
	// the bone transforms is use for
	// render thread(main thread) and update thread
	// if change the bone transforms value in loop
	// it is too slow and make not synchronized data
	// when render thread use this. so I just put change data
	// to local bone transform in loop and copy the local bone
	// transform to bone transforms. it work perface now.
	boneTransforms = localBoneTransform;
}

void FbxModel::finalizeVBO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// vertices
	vertices.bindVBO();
	vertices.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// colors
	colors.bindVBO();
	colors.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// normals
	normals.bindVBO();
	normals.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// uvs
	uvs[0].bindVBO();
	uvs[0].uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

	if (!hasAnimation)
		return;

	// vertex bone indices
	boneIndices.bindVBO();
	boneIndices.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_INT, 0, 0);

	// vertex bone weights
	boneWeights.bindVBO();
	boneWeights.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

glm::mat4 FbxModel::toGlmMatrix(FbxAMatrix & matrix)
{
	float data[16];
	data[0] = matrix.mData[0].mData[0]; data[1] = matrix.mData[0].mData[1]; data[2] = matrix.mData[0].mData[2]; data[3] = matrix.mData[0].mData[3];
	data[4] = matrix.mData[1].mData[0]; data[5] = matrix.mData[1].mData[1]; data[6] = matrix.mData[1].mData[2]; data[7] = matrix.mData[1].mData[3];
	data[8] = matrix.mData[2].mData[0]; data[9] = matrix.mData[2].mData[1]; data[10] = matrix.mData[2].mData[2]; data[11] = matrix.mData[2].mData[3];
	data[12] = matrix.mData[3].mData[0]; data[13] = matrix.mData[3].mData[1]; data[14] = matrix.mData[3].mData[2]; data[15] = matrix.mData[3].mData[3];
	return glm::mat4(data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7],
		data[8], data[9], data[10], data[11],
		data[12], data[13], data[14], data[15]);
}

glm::mat4 FbxModel::toGlmMatrix(FbxMatrix & matrix)
{
	float data[16];
	data[0] = matrix.mData[0].mData[0]; data[1] = matrix.mData[0].mData[1]; data[2] = matrix.mData[0].mData[2]; data[3] = matrix.mData[0].mData[3];
	data[4] = matrix.mData[1].mData[0]; data[5] = matrix.mData[1].mData[1]; data[6] = matrix.mData[1].mData[2]; data[7] = matrix.mData[1].mData[3];
	data[8] = matrix.mData[2].mData[0]; data[9] = matrix.mData[2].mData[1]; data[10] = matrix.mData[2].mData[2]; data[11] = matrix.mData[2].mData[3];
	data[12] = matrix.mData[3].mData[0]; data[13] = matrix.mData[3].mData[1]; data[14] = matrix.mData[3].mData[2]; data[15] = matrix.mData[3].mData[3];
	return glm::mat4(data[0], data[4], data[8], data[12],
		data[1], data[5], data[9], data[13],
		data[2], data[6], data[10], data[14],
		data[3], data[7], data[11], data[15]);
}

glm::vec4 oglt::FbxModel::toGlmVec4(FbxVector4 & fbxVec4)
{
	return vec4(fbxVec4[0], fbxVec4[1], fbxVec4[2], fbxVec4[3]);
}

glm::quat FbxModel::toGlmQuat(FbxQuaternion & fbxQuat)
{
	return quat(fbxQuat[0], fbxQuat[1], fbxQuat[2], fbxQuat[3]);
}

glm::vec3 oglt::FbxModel::toGlmVec3(FbxVector4 & fbxVec4)
{
	return vec3(fbxVec4[0], fbxVec4[1], fbxVec4[2]);
}

void FbxModel::render(int renderType)
{
	if (!loaded || !visiable)
		return;

	if (hasAnimation) {
		mutexBoneTransforms = &boneTransforms;
	}

	glBindVertexArray(vao);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FOR(i, ESZ(meshs)) {
		if (meshs[i].mtlMapMode == ALL_SAME) {
			IMaterial* material = Resource::instance()->getMaterial(meshs[i].materialId);
			if (material != NULL) {
				material->setShaderProgram(shaderProgram);
				material->useMaterial();
			}
			glDrawArrays(GL_TRIANGLES, meshs[i].startIndex, meshs[i].size);
		}
		else if (meshs[i].mtlMapMode == BY_POLYGON) {
			// test rendering with per polygon, the fps still on 1300
			FOR(j, ESZ(meshs[i].polygons)) {
				IMaterial* material = Resource::instance()->getMaterial(meshs[i].polygons[j].materialId);
				if (material != NULL) {
					material->setShaderProgram(shaderProgram);
					material->useMaterial();
				}
				glDrawArrays(GL_TRIANGLES, meshs[i].polygons[j].startIndex, meshs[i].polygons[j].size);
			}
		}
	}
	glDisable(GL_BLEND);

	mutexShaderProgram = NULL;
	mutexBoneTransforms = NULL;
}

void FbxModel::VertexBoneData::addBoneData(oglt::uint boneIndex, float weight)
{
	FOR(i, 4) {
		if (weights[i] <= 0.00001f) {
			boneIndices[i] = boneIndex;
			weights[i] = weight;
			return;
		}
	}
}
