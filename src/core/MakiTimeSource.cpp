#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

#include "SDL.h"

namespace maki
{
	namespace core
	{

		time_source_t::time_source_t()
		{
			uint64 ticksPerSecond = SDL_GetPerformanceFrequency();

			// Ticks per microsecond
			frequency = ticksPerSecond / 1e6;
			console_t::info("Clock frequency is %f ticks per microsecond", frequency);
			assert(frequency != 0 && "Platform clock doesn't give microsecond resolution");

			start = SDL_GetPerformanceCounter();
		}

		time_source_t::~time_source_t()
		{
		}

		uint64 time_source_t::GetTimeMicro()
		{
			uint64 now = SDL_GetPerformanceCounter();
			if(now < start) {
				now = start;
			}
			return (uint64)((now - start) / frequency);
		}


	} // namespace core

} // namespace maki
