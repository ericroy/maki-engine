#pragma once
#include "Maki.h"
#include "SDL.h"

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX 1
#	include <windows.h>
#endif

#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glext.h>
#include "ogl/mojodds/mojodds.h"