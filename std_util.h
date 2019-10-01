#pragma once

#define _USE_MATH_DEFINES

#include <Windows.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <time.h>
#include <math.h>
#include <map>
#include <thread>
#include <mutex>

#define FOR(q,n) for(int q=0;q<n;q++)
#define SFOR(q,s,e) for(int q=s;q<=e;q++)
#define RFOR(q,n) for(int q=n;q>=0;q--)
#define RSFOR(q,s,e) for(int q=s;q>=e;q--)

#define ESZ(elem) (int)elem.size()

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

using namespace std;

string getDirectoryPath(string sFilePath);
float getRadian(float degree);