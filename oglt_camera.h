#pragma once

#include "oglt_scene_node.h"
#include "oglt_iapp.h"

namespace oglt {
	namespace scene {
		class FlyingCamera : public SceneNode
		{
		public:
			float speed;
			float sensitivity; // How many degrees to rotate per pixel moved by mouse (nice value is 0.10)

								// Main functions
			void update(int updateMode = OGLT_UPDATEA_CAMERA_WALK | OGLT_UPDATE_CAMERA_ROTATE);
			glm::mat4* look();
			glm::vec3* getUp();
			glm::vec3* getView();

			void setMovingKeys(int a_iForw, int a_iBack, int a_iLeft, int a_iRight);
			void resetMouse();

			// Constructors
			FlyingCamera();
			FlyingCamera(oglt::IApp* app, glm::vec3 a_vEye, glm::vec3 a_vView, glm::vec3 a_vUp, float a_fSpeed, float a_fSensitivity);

		private:
			void rotateWithMouse();
			// Functions that get viewing angles
			float getAngleX(), getAngleY();

			int forwKey, backKey, leftKey, rightKey;

			glm::vec3 vView, vUp;
			glm::mat4 viewMatrix;

			oglt::IApp* app;
		};
	}
}