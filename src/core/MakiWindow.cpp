#include "core/core_stdafx.h"
#include "core/MakiWindow.h"

namespace Maki
{
	
	Window::Window(int32 width, int32 height)
		: width(width),
		height(height),
		fullscreen(false)
	{
	}

	Window::~Window()
	{
	}

} // namespace Maki