#include "core/MakiInputState.h"

namespace maki {
	namespace core {

		controller_t::controller_t() {
			set_identity_mapping();
		}

		void controller_t::map_button(button_id_t hardware_button, button_id_t virtual_button) {
			mapping_[virtual_button] = hardware_button;
		}

		void controller_t::set_identity_mapping() {
			for(uint8_t i = 0; i < button_max + 1; i++)
				mapping_[i] = i;
		}

		void controller_t::set_values(const float values[button_max + 1]) {
			memcpy(values_, values, button_max + 1);
		}

		



		player_t::player_t() {
			set_identity_mapping();
		}

		void player_t::map_key(key_id_t hardware_key, key_id_t virtual_key) {
			key_mapping_[virtual_key] = hardware_key;
		}

		void player_t::set_identity_mapping() {
			for(uint32_t i = 0; i < 256; i++)
				key_mapping_[i] = i;
		}

		void player_t::set_key_states(const key_state_report_t states[256]) {
			for(uint32_t i = 0; i < 256; i++) {
				if(key_states_[i].on == states[i].on && states[i].transition_count == 0) {
					// Nothing happened with this key
					key_states_[i].pressed = 0;
					key_states_[i].released = 0;
				} else {
					// Something happened
					// The key may have changed its 'on' state since last frame.
					// It is also possible that the key has the same 'on' state as last frame, but there was one
					// (or more) intra-frame up-down events.
					key_states_[i].on = states[i].on;
					if(states[i].on) {
						if(states[i].transition_count == 1) {
							key_states_[i].pressed = 1;
							key_states_[i].released = 0;
						} else {
							key_states_[i].pressed = 1;
							key_states_[i].released = 1;
						}
					} else {
						if(states[i].transition_count == 1) {
							key_states_[i].released = 1;
							key_states_[i].pressed = 0;
						} else {
							key_states_[i].released = 1;
							key_states_[i].pressed = 1;
						}
					}
				}
			}
		}




		void input_state_t::connect_controller(uint8_t controller_index) {
			MAKI_ASSERT(controller_count_ < max_players && "Controller index out of range");
			MAKI_ASSERT(!controllers_[controller_index].connected() && "Controller already connected");

			controllers_[controller_index].set_connected(true);
		
			// find the next free player to associate this controller with
			for(uint8_t i = 0; i < max_players; i++) {
				if(players_[i].controller() == nullptr) {
					players_[i].set_controller(&controllers_[controller_index]);
					break;
				}
			}
		}

		void input_state_t::disconnect_controller(uint8_t controller_index) {
			auto *c = &controllers_[controller_index];
			c->set_connected(false);
			for(uint8_t i = 0; i < max_players; i++) {
				if (players_[i].controller() == c) {
					players_[i].set_controller(nullptr);
					break;
				}
			}
		}

	} // namespace core
} // namespace maki
