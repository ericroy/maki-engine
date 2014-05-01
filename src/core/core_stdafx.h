#pragma once
#define MAKI_USE_SIMD 1
#define MAKI_CONSOLE_OUTPUT_ENABLED 1
#define MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD 0
#define MAKI_PROFILING 0

#define _CRT_SECURE_NO_WARNINGS 1

#include <cstdio>
#include <new>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <assert.h>
#include <functional>
#include <algorithm>


#include "core/MakiAllocator.h"
#include "core/MakiTypes.h"
#include "core/MakiMacros.h"
#include "core/MakiMoveToken.h"
#include "core/MakiConsole.h"
#include "core/MakiArray.h"
#include "core/MakiMatrix44.h"
#include "core/MakiVector2.h"
#include "core/MakiVector3.h"
#include "core/MakiVector4.h"
#include "core/MakiQuaternion.h"
#include "core/MakiHandleOrRid.h"
#include "core/MakiFrustum.h"
#include "core/MakiRect.h"
#include "core/MakiBoundingBox.h"
#include "core/MakiBoundingSphere.h"
