#pragma once

#include "oglt_irenderable.h"
#include "oglt_shader.h"
#include "oglt_vbo.h"
#include "oglt_texture.h"

namespace oglt {
	class AssimpModel : public IRenderable
	{
	public:
		bool loadModelFromFile(char* sFilePath);

		virtual void render(int renderType = OGLT_RENDER_SELF);
		AssimpModel();
	private:
		void finalizeVBO();

		bool loaded;
		VertexBufferObject vboData;
		uint vao;
		vector<Texture> textures;
		vector<uint> meshStartIndices;
		vector<uint> meshSizes;
		vector<uint> materialIndices;
		int numMaterials;
	};
}
