#pragma once
#include <cstdio>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cmath>
#include <assert.h>
#include <functional>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#	define _WINSOCKAPI_
#endif
#include "Maki.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "framework/MakiMacros.h"
#include "framework/MakiTween.h"
#include "framework/MakiEntity.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "SDL2.lib")

#if _DEBUG
#	if defined(_WIN32) || defined(_WIN64)
#		pragma comment(lib, "ws2_32.lib")
#	endif
#	pragma comment(lib, "BulletCollision_vs2010_debug")
#	pragma comment(lib, "BulletDynamics_vs2010_debug")
#	pragma comment(lib, "LinearMath_vs2010_debug")
#else
#	pragma comment(lib, "BulletCollision_vs2010")
#	pragma comment(lib, "BulletDynamics_vs2010")
#	pragma comment(lib, "LinearMath_vs2010")
#endif


#define MAKI_API __declspec(dllexport)