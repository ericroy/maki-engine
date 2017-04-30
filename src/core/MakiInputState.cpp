#include "core/core_stdafx.h"
#include "core/MakiInputState.h"

namespace maki
{
	namespace core
	{

		input_state_t::input_state_t()
			: controller_count_(0)
		{
			memset(controllers_, 0, sizeof(controllers_));
			memset(players_, 0, sizeof(players_));

			for(uint8_t i = 0; i < max_players_; i++) {
				players_[i].set_identity_mapping();
				controllers_[i].set_identity_mapping();
			}
		}

		input_state_t::~input_state_t()
		{
		}

		void input_state_t::connect_controller(uint8_t controller_index)
		{
			assert(controller_count_ < max_players_);
			if(!controllers_[controller_index].connected_) {
				controllers_[controller_index].connected_ = true;
			
				// find the next free player to associate this controller with
				for(uint8_t player_index = 0; player_index < max_players_; player_index++) {
					if(players_[player_index].controller_ == nullptr) {
						players_[player_index].controller_ = &controllers_[controller_index];
						controllers_[controller_index].player_index_ = player_index;
						return;
					}
				}
			}
		}

		void input_state_t::disconnect_controller(uint8_t controller_index)
		{
			controller_t &c = controllers_[controller_index];
			c.connected_ = false;
			players_[c.player_index_].controller_ = nullptr;
			c.player_index_ = (uint8_t)-1;
		}

	} // namespace core

} // namespace maki
