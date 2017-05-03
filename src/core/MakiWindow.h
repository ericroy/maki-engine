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
			
			inline float get_aspect() {
				return height_ == 0 ? 1.0f : width_ / (float)height_;
			}

			inline int32_t width() const {
				return width_;
			}

			inline int32_t height() const {
				return height_;
			}

			inline bool fullscreen() const {
				return fullscreen_;
			}

			inline void *handle() const {
				return static_cast<void *>(window_);
			}

		private:
			void connect_game_controller(int32_t i); 

			inline int32_t get_controller_index(int32_t instance_id) {
				for (int32_t i = 0; i < input_state_t::max_players; i++) {
					if (controllers_[i].instance_id == instance_id)
						return i;
				}
				return -1;
			}

		private:
			int32_t width_ = 0;
			int32_t height_ = 0;
			bool fullscreen_ = false;
			SDL_Window *window_ = nullptr;

			key_state_report_t key_states_[256];

			struct {
				int32_t instance_id = (int32_t)-1;
				SDL_GameController *handle = nullptr;
				float values[button_max + 1] = {};
			} controllers_[input_state_t::max_players];
		};

	} // namespace core
} // namespace maki
