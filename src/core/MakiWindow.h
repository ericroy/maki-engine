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

namespace maki
{
	namespace core
	{
		class config_t;
		class engine_t;

		class window_t
		{
		public:
			window_t(render_core_t::type_t renderCoreType, const config_t *config);
			virtual ~window_t();
			int Pump(engine_t *engine);
			void poll_input(input_state_t *state);
			inline float get_aspect() { return height == 0 ? (float)width : width / (float)height; };

		private:
			void ConnectGameController(int32 i); 
			inline int32 GetControllerIndex(int32 instanceId)
			{
				for(int32 i = 0; i < input_state_t::max_players_; i++) {
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
			input_state_t::key_state_report_t keyStates[256];
			SDL_GameController *controllerHandles[input_state_t::max_players_];
			input_state_t::controller_t controllers[input_state_t::max_players_];
			int32 controllerInstanceIds[input_state_t::max_players_];
		};

	} // namespace core

} // namespace maki
