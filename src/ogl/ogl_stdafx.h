#pragma once
#include "Maki.h"
#include "SDL.h"

#if defined(_WIN32) || defined(_WIN64)
#	define NOMINMAX 1
#	include <windows.h>
#endif


#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
// #include <GL/glext.h>

#include "ogl/mojodds/mojodds.h"