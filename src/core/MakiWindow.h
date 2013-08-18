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
			void ConnectGameController(int32 i); 
			inline int32 GetControllerIndex(int32 instanceId)
			{
				for(int32 i = 0; i < InputState::MAX_PLAYERS; i++) {
					if(controllerInstanceIds[i] == instanceId) { return i; }
				}
				return -1;
			}

		public:
			int32 width;
			int32 height;
			bool fullscreen;
			SDL_Window *window;

		private:
			InputState::KeyStateReport keyStates[256];
			SDL_GameController *controllerHandles[InputState::MAX_PLAYERS];
			InputState::Controller controllers[InputState::MAX_PLAYERS];
			int32 controllerInstanceIds[InputState::MAX_PLAYERS];
		};

	} // namespace Core

} // namespace Maki