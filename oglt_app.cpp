#include "oglt_app.h"

#include "glut_backend.h"
#include "alut_backend.h"

#include "render_scene.h"

#include <glut.h>
#include <glm\gtc\matrix_transform.hpp>

using namespace oglt;

bool IApp::keyStates[256] = { false };
bool IApp::oneKeyStats[256] = { false };
Cursor IApp::cursor;

OgltApp::OgltApp() {
	
}

OgltApp::~OgltApp() {
	scene::releaseScene(this);
}

void OgltApp::init(int argc, char** argv, int ogltOptions) {
	IApp::init();
	glutBackendInit(argc, argv, ogltOptions);
	alutBackendInit(argc, argv);
}

bool OgltApp::createWindow(uint width, uint height, const char* title, int ogltOptions) {
	return glutBackendCreateWindow(width, height, title, ogltOptions);
}

void OgltApp::updateScene()
{
	updateTimer();
	scene::updateScene(this);
	if (deltaTime < 0.001) {
		// if update time is too quickly
		// let this thread sleep 1 millisecond
		this_thread::sleep_for(chrono::milliseconds(1));
	}
}

void OgltApp::releaseScene()
{
	scene::releaseScene(this);
}

void OgltApp::run() {
	scene::initScene(this);
	glutBackendRun(this);
}

void OgltApp::renderScene() {
	updateFrameTimer();
	scene::renderScene(this);
}

void OgltApp::keyboard(OGLT_KEY key, OGLT_KEY_STATE state){
	switch (state) {
	case OGLT_KEY_PRESS:
		if (!keyStates[key]) {
			oneKeyStats[key] = true;
		}
		else {
			oneKeyStats[key] = false;
		}

		keyStates[key] = true;
		if (key == 'q' || key == OGLT_KEY_ESCAPE) {
			glutBackendExit();
			alutBackendExit();
			exit(0);
		}
		break;
	case OGLT_KEY_RELEASE:
		keyStates[key] = false;
		oneKeyStats[key] = false;
		break;
	}
}

void OgltApp::mouse(OGLT_BUTTON button, OGLT_BUTTON_STATE state, int x, int y) {
	
}

void OgltApp::mouseMotion(int x, int y) {
	cursor.x = x;
	cursor.y = y;
}

void OgltApp::reshape(uint width, uint height)
{
	viewportWidth = width;
	viewportHeight = height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(45.0f, (float)width / (float)height, 0.5f, 35000.0f);
	orth = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
}

void oglt::OgltApp::swapBuffers()
{
	glutBackendSwapBuffers();
}

void OgltApp::setCursor(int x, int y)
{
	cursor.x = x;
	cursor.y = y;
	glutBackendSetCursor(x, y);
}

void oglt::OgltApp::setCursor(OGLT_CURSOR_TYPE cursorType)
{
	glutBackendSetCursor(cursorType);
}

void OgltApp::getCursor(int & x, int & y)
{
	x = cursor.x;
	y = cursor.y;
}

bool OgltApp::key(int ikey)
{
	return keyStates[ikey];
}

bool OgltApp::oneKey(int ikey)
{
	if (oneKeyStats[ikey]) {
		oneKeyStats[ikey] = false;
		return true;
	}
	return false;
}
