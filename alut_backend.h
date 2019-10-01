#pragma once

namespace oglt {
	void alutBackendInit(int argc, char **argv);
	void alutBackendExit();
	void alutBackendPlayFile(const char* fileName, bool looping = true);
}