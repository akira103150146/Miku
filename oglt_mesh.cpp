#include "oglt_mesh.h"

oglt::Mesh::Mesh()
{
	startIndex = 0;
	size = 0;
	materialId = OGLT_INVALID_MATERIAL_ID;
	mtlMapMode = NONE;
}

oglt::Mesh::~Mesh()
{
}

void oglt::Mesh::sortTrianglesByMtl()
{
}

void oglt::Mesh::buildPolygonsPerMtl(bool clearTriangles)
{
	if (ESZ(triangles) <= 0) {
		fprintf(stderr, "Error: no triangles data can build the polygons.");
		return;
	}

	Polygon polygon;
	polygon.startIndex = triangles[0].startIndex;
	polygon.materialId = triangles[0].materialId;
	FOR(i, ESZ(triangles)) {
		if (polygon.materialId != triangles[i].materialId) {
			polygon.size = triangles[i].startIndex - polygon.startIndex;
			polygons.push_back(polygon);
			polygon.startIndex = triangles[i].startIndex;
			polygon.materialId = triangles[i].materialId;
		}
	}

	polygon.size = triangles[ESZ(triangles) - 1].startIndex - polygon.startIndex + 3;
	// push the remain polygon
	polygons.push_back(polygon);

	if (clearTriangles) {
		triangles.clear();
	}
}
