#include "core/MakiTimeSource.h"
#include "core/MakiConsole.h"
#include "SDL.h"

namespace maki {
	namespace core {

		time_source_t::time_source_t() {
			uint64_t ticks_per_second = SDL_GetPerformanceFrequency();

			// Ticks per microsecond
			frequency_ = ticks_per_second / 1e6;
			console_t::info("Clock frequency is %f ticks per microsecond", frequency_);
			MAKI_ASSERT(frequency_ != 0 && "Platform clock doesn't give microsecond resolution");

			start_ = SDL_GetPerformanceCounter();
		}

		uint64_t time_source_t::get_time_micro() {
			uint64_t now = SDL_GetPerformanceCounter();
			if(now < start_)
				now = start_;
			return (uint64_t)((now - start_) / frequency_);
		}

	} // namespace core
} // namespace maki
