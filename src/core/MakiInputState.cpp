#include "core/MakiInputState.h"

namespace maki {
	namespace core {

		input_state_t::input_state_t() {
			for(uint8_t i = 0; i < max_players; i++) {
				players_[i].set_identity_mapping();
				controllers_[i].set_identity_mapping();
			}
		}

		void input_state_t::connect_controller(uint8_t controller_index) {
			MAKI_ASSERT(controller_count_ < max_players);
			if(!controllers_[controller_index].connected) {
				controllers_[controller_index].connected = true;
			
				// find the next free player to associate this controller with
				for(uint8_t player_index = 0; player_index < max_players; player_index++) {
					if(players_[player_index].controller == nullptr) {
						players_[player_index].controller = &controllers_[controller_index];
						controllers_[controller_index].player_index = player_index;
						return;
					}
				}
			}
		}

		void input_state_t::disconnect_controller(uint8_t controller_index) {
			auto &c = controllers_[controller_index];
			c.connected = false;
			players_[c.player_index].controller = nullptr;
			c.player_index = (uint8_t)-1;
		}

	} // namespace core
} // namespace maki
