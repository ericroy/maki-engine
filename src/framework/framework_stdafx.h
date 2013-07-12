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
