#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMessage.h"
#include "framework/hash/hash.h"

namespace Maki
{
	namespace Framework
	{

		MAKI_DEFINE_MESSAGE(Message_Ping, "ping");
		MAKI_DEFINE_MESSAGE(Message_Pong, "pong");
		MAKI_DEFINE_MESSAGE(Message_PlayTrack, "play_track");
		
	} // namespace Framework

} // namespace Maki