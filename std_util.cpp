#include "std_util.h"

/*-----------------------------------------------

Name:	getDirectoryPath

Params:	sFilePath - guess ^^

Result: Returns directory name only from filepath.

/*---------------------------------------------*/

string getDirectoryPath(string sFilePath)
{
	// Get directory path
	string sDirectory = "";
	RFOR(i, ESZ(sFilePath) - 1)if (sFilePath[i] == '\\' || sFilePath[i] == '/')
	{
		sDirectory = sFilePath.substr(0, i + 1);
		break;
	}
	return sDirectory;
}

float getRadian(float degree) {
	return (degree / 180.0f) * M_PI;
}