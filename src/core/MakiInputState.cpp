#include "core/core_stdafx.h"
#include "core/MakiInputState.h"

namespace maki
{
	namespace core
	{

		input_state_t::input_state_t()
			: controllerCount(0)
		{
			memset(controllers, 0, sizeof(controllers));
			memset(players, 0, sizeof(players));

			for(uint8 i = 0; i < max_players_; i++) {
				players[i].set_identity_mapping();
				controllers[i].set_identity_mapping();
			}
		}

		input_state_t::~input_state_t()
		{
		}

		void input_state_t::ConnectController(uint8 controllerIndex)
		{
			assert(controllerCount < max_players_);
			if(!controllers[controllerIndex].connected) {
				controllers[controllerIndex].connected = true;
			
				// find the next free player to associate this controller with
				for(uint8 playerIndex = 0; playerIndex < max_players_; playerIndex++) {
					if(players[playerIndex].controller == nullptr) {
						players[playerIndex].controller = &controllers[controllerIndex];
						controllers[controllerIndex].playerIndex = playerIndex;
						return;
					}
				}
			}
		}

		void input_state_t::DisconnectController(uint8 controllerIndex)
		{
			controller_t &c = controllers[controllerIndex];
			c.connected = false;
			players[c.playerIndex].controller = nullptr;
			c.playerIndex = (uint8)-1;
		}

	} // namespace core

} // namespace maki
