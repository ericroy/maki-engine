#pragma once

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef EXPORT_MAKI_D3DRENDERCORE_API
#	define MAKI_D3DRENDERCORE_API __declspec(dllexport)
#	define MAKI_D3DRENDERCORE_API_EXTERN
#else
#	define MAKI_D3DRENDERCORE_API __declspec(dllimport)
#	define MAKI_D3DRENDERCORE_API_EXTERN
#endif

#include "Maki.h"
#include <windows.h>
#include <d3d11.h>