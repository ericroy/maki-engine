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
			window_t(render_core_t::type_t render_core_type, const config_t *config);
			virtual ~window_t();
			int pump(engine_t *engine);
			void poll_input(input_state_t *state);
			inline float get_aspect() { return height_ == 0 ? (float)width_ : width_ / (float)height_; };

		private:
			void connect_game_controller(int32 i); 
			inline int32 get_controller_index(int32 instance_id)
			{
				for(int32 i = 0; i < input_state_t::max_players_; i++) {
					if(controller_instance_ids_[i] == instance_id) { return i; }
				}
				return -1;
			}

		public:
			int32 width_;
			int32 height_;
			bool fullscreen_;
			SDL_Window *window_;

		private:
			input_state_t::key_state_report_t key_states_[256];
			SDL_GameController *controller_handles_[input_state_t::max_players_];
			input_state_t::controller_t controllers_[input_state_t::max_players_];
			int32 controller_instance_ids_[input_state_t::max_players_];
		};

	} // namespace core

} // namespace maki
