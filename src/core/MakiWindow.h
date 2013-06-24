#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{
		class InputState;

		class Window
		{
		public:
			Window(int32 width, int32 height);
			virtual ~Window();
			virtual void PollInput(InputState *state) = 0;
			inline float GetAspect() { return height == 0 ? (float)width : width / (float)height; };

		public:
			int32 width;
			int32 height;
			bool fullscreen;
		};

	} // namespace Core

} // namespace Maki