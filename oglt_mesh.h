#pragma once

#include "std_util.h"
#include "oglt_util.h"
#include <glm/glm.hpp>

namespace oglt {
	enum MaterialMappingMode {
		BY_POLYGON, ALL_SAME, NONE
	};

	// the helper data struct for mesh.
	// usually the triangle struct is used
	// to build the polygon.
	// because use per triangle to rendering
	// the performance will so poor
	struct Triangle {
		uint vertexIndices[3];
		uint startIndex;
		uint materialId;
	};

	// the helper data struct for mesh.
	// polygon contains one more triangles.
	// so it's always use triangles to build polygon
	// when the mesh's material mapping mode
	// is by polygon. the polygon will be used.
	struct Polygon {
		uint startIndex;
		uint size;
		uint materialId;
	};

	// the mesh claa use for the model or some can be
	// rendered class. all data except material store in mesh is the index
	// from real data store in gpu.
	// everytime write a renderable class use mesh
	// has to build the relationship with mesh and the data in gpu.
	class Mesh {
	public:
		Mesh();
		~Mesh();

		void sortTrianglesByMtl();
		// build polygons from triangles by per material
		// after build polygons if the clearTriangles param is true
		// this function will auto clear all triangles data.
		void buildPolygonsPerMtl(bool clearTriangles = true);

		uint startIndex;
		uint size;
		uint materialId;
		vector<Triangle> triangles;
		vector<Polygon> polygons;

		MaterialMappingMode mtlMapMode;
	private:
	};
}