#include "std_util.h"

#include "oglt_app.h"

using namespace oglt;

int main(int argc, char** argv) {
	OgltApp app;
	app.init(argc, argv, OGLT_DEPTH);
	if (!app.createWindow(800, 600, "my oglt window", OGLT_NONE)) {
		fprintf(stderr, "oglt app can not create window\n");
		return -1;
	}
	app.run();
	return 0;
}