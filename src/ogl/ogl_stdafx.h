#pragma once
#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifdef EXPORT_MAKI_OGLRENDERCORE_API
#	define MAKI_OGLRENDERCORE_API __declspec(dllexport)
#	define MAKI_OGLRENDERCORE_API_EXTERN
#else
#	define MAKI_OGLRENDERCORE_API __declspec(dllimport)
#	define MAKI_OGLRENDERCORE_API_EXTERN
#endif

#include "Maki.h"
#include "SDL.h"
#if defined(_WIN32) || defined(_WIN64)
#	include <windows.h>
#endif
#include <gl/GL.h>
#include <gl/GLU.h>
#include "ogl/glext.h"