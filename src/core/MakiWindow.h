#pragma once
#include "core/core_stdafx.h"
#include "core/MakiInputState.h"
#include "core/MakiRenderCore.h"

#include "SDL.h"
#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX 1
#include <Windows.h>
#include <Xinput.h>
#endif

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

		private:
			void PollRegular(InputState *state, uint32 joystickIndex);
			void PollXInput(InputState *state, uint32 joystickIndex);
			void DiscoverJoysticks();

		public:
			int32 width;
			int32 height;
			bool fullscreen;
			SDL_Window *window;

		private:
			InputState::KeyStateReport keyStates[256];
			uint32 joystickConnectedFlags;
			SDL_Joystick *joysticks[InputState::MAX_PLAYERS];
			bool joystickIsXInput[InputState::MAX_PLAYERS];

#if defined(_WIN32) || defined(_WIN64)
			uint32 joystickIndexToXInputIndex[InputState::MAX_PLAYERS];
			XINPUT_STATE controllers[InputState::MAX_PLAYERS];
#endif
		};

	} // namespace Core

} // namespace Maki