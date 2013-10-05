#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMessage.h"
#include "framework/hash/hash.h"

namespace Maki
{
	namespace Framework
	{

		MAKI_DEFINE_HASH(Message_Ping, "ping");
		MAKI_DEFINE_HASH(Message_Pong, "pong");
		MAKI_DEFINE_HASH(Message_PlayTrack, "play_track");
		MAKI_DEFINE_HASH(Message_TrackComplete, "track_complete");
		
	} // namespace Framework

} // namespace Maki