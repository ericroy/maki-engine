#include "core/core_stdafx.h"
#include "core/MakiInputState.h"

namespace Maki
{

	InputState::InputState()
		: controllerCount(0)
	{
		memset(controllers, 0, sizeof(controllers));
		memset(players, 0, sizeof(players));

		for(uint8 i = 0; i < MAX_PLAYERS; i++) {
			players[i].SetMappingIdentity();
			controllers[i].SetMappingIdentity();
		}
	}

	InputState::~InputState()
	{
	}

	void InputState::ConnectController(uint8 controllerIndex)
	{
		assert(controllerCount < MAX_PLAYERS);
		if(!controllers[controllerIndex].connected) {
			controllers[controllerIndex].connected = true;
			
			// Find the next free player to associate this controller with
			for(uint8 playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
				if(players[playerIndex].controller == nullptr) {
					players[playerIndex].controller = &controllers[controllerIndex];
					controllers[controllerIndex].playerIndex = playerIndex;
					return;
				}
			}
		}
	}

	void InputState::DisconnectController(uint8 controllerIndex)
	{
		Controller &c = controllers[controllerIndex];
		c.connected = false;
		players[c.playerIndex].controller = nullptr;
		c.playerIndex = (uint8)-1;
	}

} // namespace Maki