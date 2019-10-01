#pragma once
#include "glut_backend.h"
#include "oglt_iapp.h"

namespace oglt {
	class OgltApp : public IApp, ICallback {
	public:
		OgltApp();
		~OgltApp();
		void init(int argc, char** argv, int ogltOptions);
		bool createWindow(uint width, uint height, const char* title, int ogltOptions);
		void run();

	    virtual void renderScene();
		virtual void updateScene();
		virtual void releaseScene();
		virtual void keyboard(OGLT_KEY key, OGLT_KEY_STATE state);
		virtual void mouse(OGLT_BUTTON button, OGLT_BUTTON_STATE state, int x, int y);
		virtual void mouseMotion(int x, int y);
		virtual void reshape(uint width, uint height);

		virtual void swapBuffers();

		virtual void setCursor(int x, int y);
		virtual void setCursor(OGLT_CURSOR_TYPE cursorType);
		virtual void getCursor(int& x, int& y);

		virtual bool key(int ikey);
		virtual bool oneKey(int ikey);
	};
}