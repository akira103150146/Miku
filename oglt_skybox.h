#pragma once

#include "oglt_util.h"
#include "oglt_scene_node.h"
#include "oglt_texture.h"
#include "oglt_vbo.h"

/********************************

Class:		Skybox

Purpose:	Class for using skybox.

********************************/

namespace oglt {
	namespace scene {
		class Skybox : public SceneNode
		{
		public:
			void load(string a_sDirectory, string a_sFront, string a_sBack, string a_sLeft, string a_sRight, string a_sTop, string a_sBottom);
			virtual void render(int renderType = OGLT_RENDER_SELF);

			bool operator==(const Skybox& other);

			void deleteSkybox();
		private:
			uint vao;
			VertexBufferObject vboData;
			Texture textures[6];
			string sDirectory;
			string sFront, sBack, sLeft, sRight, sTop, sBottom;
		};
	}
}