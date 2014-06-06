#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

#include "SDL.h"

namespace Maki
{
	namespace Core
	{

		TimeSource::TimeSource()
		{
			uint64 ticksPerSecond = SDL_GetPerformanceFrequency();

			// Ticks per microsecond
			frequency = ticksPerSecond / 1e6;
			Console::Info("Clock frequency is %f ticks per microsecond", frequency);
			assert(frequency != 0 && "Platform clock doesn't give microsecond resolution");

			start = SDL_GetPerformanceCounter();
		}

		TimeSource::~TimeSource()
		{
		}

		uint64 TimeSource::GetTimeMicro()
		{
			uint64 now = SDL_GetPerformanceCounter();
			if(now < start) {
				now = start;
			}
			return (uint64)((now - start) / frequency);
		}


	} // namespace Core

} // namespace Maki