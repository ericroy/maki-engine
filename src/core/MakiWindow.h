#pragma once
#include "core/MakiTypes.h"
#include "core/MakiInputState.h"
#include "core/MakiRenderCore.h"
#include "SDL.h"

namespace maki {
	namespace core {
		class config_t;
		class engine_t;

		class window_t {
		public:
			window_t(render_core_t::type_t render_core_type, const config_t *config);
			virtual ~window_t();
			int pump(engine_t *engine);
			void poll_input(input_state_t *state);
			inline float get_aspect() { return height == 0 ? (float)width : width / (float)height; };

		private:
			void connect_game_controller(int32_t i); 

			inline int32_t get_controller_index(int32_t instance_id) {
				for(int32_t i = 0; i < input_state_t::max_players; i++)
					if(controller_instance_ids_[i] == instance_id) { return i; }
				return -1;
			}

		public:
			int32_t width = 0;
			int32_t height = 0;
			bool fullscreen = false;
			SDL_Window *window = nullptr;

		private:
			input_state_t::key_state_report_t key_states_[256] = {};
			SDL_GameController *controller_handles_[input_state_t::max_players] = {};
			input_state_t::controller_t controllers_[input_state_t::max_players] = {};
			int32_t controller_instance_ids_[input_state_t::max_players] = {};
		};

	} // namespace core
} // namespace maki
