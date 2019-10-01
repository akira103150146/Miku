#include "glut_backend.h"

#include <GL\glew.h>

#include <freeglut.h>

#include "std_util.h"

using namespace oglt;

static ICallback* spCallback = NULL;
static int sOgltOptions = 0;

static thread* spUpdateThread = NULL;
static bool exited = false;

void initCallback();
static void updateScene();

void oglt::glutBackendInit(int argc, char** argv, int ogltOptions) {
	glutInit(&argc, argv);

	uint DisplayMode = GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE;

	if (OGLT_DEPTH & ogltOptions) {
		DisplayMode |= GLUT_DEPTH;
	}

	if (OGLT_STENCIL & ogltOptions) {
		DisplayMode |= GLUT_STENCIL;
	}
	
	sOgltOptions = ogltOptions;

	glutInitDisplayMode(DisplayMode);
}

bool oglt::glutBackendCreateWindow(uint width, uint height, const char* title, int ogltOptions) {
	glutInitWindowSize(width, height);
	glutCreateWindow(title);

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return false;
	}

	glutCreateMenu(NULL);
	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(width, height);

	return true;
}

void oglt::glutBackendRun(ICallback* pCallback) {
	if (!pCallback) {
		fprintf(stderr, "%s : callback not specified!\n", __FUNCTION__);
		return;
	}

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_MULTISAMPLE);

	if (OGLT_DEPTH & sOgltOptions) {
		glEnable(GL_DEPTH_TEST);
	}

	glutSetCursor(GLUT_CURSOR_NONE);

	spCallback = pCallback;
	initCallback();
	spUpdateThread = new thread(updateScene);
	glutMainLoop();
}

void oglt::glutBackendExit()
{
	// exit glut backend now
	// use leave main loop for render nothing
	exited = true;
	glutLeaveMainLoop();
	// waiting for update thread exit
	// and delete it
	spUpdateThread->join();
	delete spUpdateThread;
	// release the resources in object
	// of spCallback pointer
	spCallback->releaseScene();
}

void oglt::glutBackendSetCursor(int x, int y)
{
	glutWarpPointer(x, y);
}

void oglt::glutBackendSetCursor(OGLT_CURSOR_TYPE cursorType)
{
	int glutCurType = GLUT_CURSOR_LEFT_ARROW;
	switch (cursorType) {
	case OGLT_CURSOR_ARROW:
		glutCurType = GLUT_CURSOR_LEFT_ARROW;
		break;
	case OGLT_CURSOR_NONE:
		glutCurType = GLUT_CURSOR_NONE;
		break;
	default:
		cout << "use oglt undefine cursor type" << endl;
	}
	glutSetCursor(glutCurType);
}

void oglt::glutBackendSwapBuffers() {
	glutSwapBuffers();
}

static OGLT_BUTTON toOgltButton(int glutMouseButton) {
	switch (glutMouseButton) {
	case GLUT_RIGHT_BUTTON:
		return OGLT_BUTTON_RIGHT;
	case GLUT_MIDDLE_BUTTON:
		return OGLT_BUTTON_MIDDLE;
	case GLUT_LEFT_BUTTON:
		return OGLT_BUTTON_LEFT;
	default:
		cout << "use oglt undefine button" << endl;
	}
	return OGLT_BUTTON_UNDIFINED;
}

static OGLT_BUTTON_STATE toOgltButtonState(int glutState) {
	switch (glutState) {
	case GLUT_DOWN:
		return OGLT_BUTTON_DOWN;
	case GLUT_UP:
		return OGLT_BUTTON_UP;
	default:
		cout << "use oglt undefine button state" << endl;
	}
	return OGLT_BUTTON_STATE_UNDEFINED;
}

static OGLT_KEY toOgltSpecialKey(int glutKey) {
	switch (glutKey) {
	case GLUT_KEY_F1:        
			return OGLT_KEY_F1;
		case GLUT_KEY_F2:        
			return OGLT_KEY_F2;
		case GLUT_KEY_F3:       
			return OGLT_KEY_F3;
		case GLUT_KEY_F4:   
			return OGLT_KEY_F4;
		case GLUT_KEY_F5:      
			return OGLT_KEY_F5;
		case GLUT_KEY_F6:     
			return OGLT_KEY_F6;
		case GLUT_KEY_F7:     
			return OGLT_KEY_F7;
		case GLUT_KEY_F8:     
			return OGLT_KEY_F8;
		case GLUT_KEY_F9:     
			return OGLT_KEY_F9;
		case GLUT_KEY_F10:    
			return OGLT_KEY_F10;
		case GLUT_KEY_F11:   
			return OGLT_KEY_F11;
		case GLUT_KEY_F12:    
			return OGLT_KEY_F12;
		case GLUT_KEY_LEFT:         
			return OGLT_KEY_LEFT;
		case GLUT_KEY_UP:         
			return OGLT_KEY_UP;
		case GLUT_KEY_RIGHT:         
			return OGLT_KEY_RIGHT;
		case GLUT_KEY_DOWN:        
			return OGLT_KEY_DOWN;            
		case GLUT_KEY_PAGE_UP:   
			return OGLT_KEY_PAGE_UP;
		case GLUT_KEY_PAGE_DOWN:      
			return OGLT_KEY_PAGE_DOWN;
		case GLUT_KEY_HOME:    
			return OGLT_KEY_HOME;
		case GLUT_KEY_END:     
			return OGLT_KEY_END;	
		case GLUT_KEY_INSERT:         
            return OGLT_KEY_INSERT;
		case 27:
			return OGLT_KEY_ESCAPE;
	}
	return OGLT_KEY_UNDEFINED;
}

static void renderScene() {
	spCallback->renderScene();
}

static void updateScene() {
	while (!exited) {
		spCallback->updateScene();
	}
}

static void idle() {
	spCallback->renderScene();
}

static void mouse(int glutButton, int glutState, int x, int y) {
	OGLT_BUTTON ogltButton = toOgltButton(glutButton);
	OGLT_BUTTON_STATE ogltState = toOgltButtonState(glutState);
	spCallback->mouse(ogltButton, ogltState, x, y);
	TwEventMouseButtonGLUT(glutButton, glutState, x, y);
}

static void mouseMotion(int x, int y) {
	spCallback->mouseMotion(x, y);
	TwEventMouseMotionGLUT(x, y);
}

static void keyboard(unsigned char glutKey, int x, int y) {
	if (
		((glutKey >= '+') && (glutKey <= '9')) ||
		((glutKey >= 'A') && (glutKey <= 'Z')) ||
		((glutKey >= 'a') && (glutKey <= 'z'))
		) {
		OGLT_KEY key = (OGLT_KEY)glutKey;
		spCallback->keyboard(key);
	}
	else {
		OGLT_KEY key = toOgltSpecialKey(glutKey);
		if (key != OGLT_KEY_UNDEFINED) {
			spCallback->keyboard(key);
		}
		else {
			cout << "Error: use oglt undefined key" << endl;
		}
	}
	TwEventKeyboardGLUT(glutKey, x, y);
}

static void keyboardUp(unsigned char glutKey, int x, int y) {
	if (
		((glutKey >= '+') && (glutKey <= '9')) ||
		((glutKey >= 'A') && (glutKey <= 'Z')) ||
		((glutKey >= 'a') && (glutKey <= 'z'))
		) {
		OGLT_KEY key = (OGLT_KEY)glutKey;
		spCallback->keyboard(key, OGLT_KEY_RELEASE);
	}
	else {
		cout << "Error: use oglt undefined key" << endl;
	}
}

static void reshape(int width, int height) {
	spCallback->reshape(width, height);
	TwWindowSize(width, height);
}

void initCallback() {
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutMouseFunc(mouse);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutReshapeFunc(reshape);
}