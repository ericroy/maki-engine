#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

#include "SDL.h"

namespace Maki
{
	namespace Core
	{

		TimeSource::TimeSource()
		{
			uint64 countsPerSecond = SDL_GetPerformanceFrequency();
			// Counts per millisecond
			frequency = countsPerSecond / 1000.0;
			start = SDL_GetPerformanceCounter();
		}

		TimeSource::~TimeSource()
		{
		}

		uint64 TimeSource::GetTimeMillis()
		{
			uint64 now = SDL_GetPerformanceCounter();
			if(now < start) {
				now = start;
			}
			return uint64((now - start) / frequency);
		}


	} // namespace Core

} // namespace Maki