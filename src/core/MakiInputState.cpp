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

			for(uint8 i = 0; i < max_players_; i++) {
				players_[i].set_identity_mapping();
				controllers_[i].set_identity_mapping();
			}
		}

		input_state_t::~input_state_t()
		{
		}

		void input_state_t::connect_controller(uint8 controllerIndex)
		{
			assert(controller_count_ < max_players_);
			if(!controllers_[controllerIndex].connected_) {
				controllers_[controllerIndex].connected_ = true;
			
				// find the next free player to associate this controller with
				for(uint8 playerIndex = 0; playerIndex < max_players_; playerIndex++) {
					if(players_[playerIndex].controller_ == nullptr) {
						players_[playerIndex].controller_ = &controllers_[controllerIndex];
						controllers_[controllerIndex].player_index_ = playerIndex;
						return;
					}
				}
			}
		}

		void input_state_t::DisconnectController(uint8 controllerIndex)
		{
			controller_t &c = controllers_[controllerIndex];
			c.connected_ = false;
			players_[c.player_index_].controller_ = nullptr;
			c.player_index_ = (uint8)-1;
		}

	} // namespace core

} // namespace maki
