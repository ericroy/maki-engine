#pragma once
#include "core/core_stdafx.h"
#include "core/MakiInputState.h"
#include "core/MakiRenderCore.h"

#include "SDL.h"

namespace Maki
{
	namespace Core
	{
		class Config;
		class Engine;

		class Window
		{
		public:
			Window(RenderCore::Type renderCoreType, const Config *config);
			virtual ~Window();
			int Pump(Engine *engine);
			void PollInput(InputState *state);
			inline float GetAspect() { return height == 0 ? (float)width : width / (float)height; };

		public:
			int32 width;
			int32 height;
			bool fullscreen;
			SDL_Window *window;

		private:
			InputState::KeyStateReport keyStates[256];
			uint32 joystickConnectedFlags;
			SDL_Joystick *joysticks[InputState::MAX_PLAYERS];
		};

	} // namespace Core

} // namespace Maki