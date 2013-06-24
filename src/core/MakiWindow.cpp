#include "core/core_stdafx.h"
#include "core/MakiWindow.h"

namespace Maki
{
	namespace Core
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

	} // namespace Core

} // namespace Maki