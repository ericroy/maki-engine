#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

#include "SDL.h"

namespace maki
{
	namespace core
	{

		time_source_t::time_source_t()
		{
			uint64 ticks_per_second = SDL_GetPerformanceFrequency();

			// Ticks per microsecond
			frequency_ = ticks_per_second / 1e6;
			console_t::info("Clock frequency_ is %f ticks per microsecond", frequency_);
			assert(frequency_ != 0 && "Platform clock doesn't give microsecond resolution");

			start_ = SDL_GetPerformanceCounter();
		}

		time_source_t::~time_source_t()
		{
		}

		uint64 time_source_t::get_time_micro()
		{
			uint64 now = SDL_GetPerformanceCounter();
			if(now < start_) {
				now = start_;
			}
			return (uint64)((now - start_) / frequency_);
		}


	} // namespace core

} // namespace maki
