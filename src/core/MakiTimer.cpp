#include "core/core_stdafx.h"
#include "core/MakiTimer.h"
#include "core/MakiTimeSource.h"

namespace Maki
{
	namespace Core
	{

		Timer::Timer(TimeSource *source)
			: source(source)
		{
			lastTime = source->GetTimeMicro();

			updateCount = 0;
			averageFps = 0.0f;

			deltaSeconds = 0.0f;
			deltaMicros = 0;

			elapsedSeconds = 0.0;
			elapsedMicros = 0;
		}

		void Timer::UpdateHistory()
		{
			uint32 index = updateCount % N_FRAME_AVERAGE;

			// If N frames have gone by, calculate new average values
			if(updateCount > N_FRAME_AVERAGE && index == 0) {
				float averageDelta = 0;
				for(uint32 i = 0; i < N_FRAME_AVERAGE; i++ ) {
					averageDelta += deltaHistory[i];
				}
				averageDelta /= N_FRAME_AVERAGE;
				averageFps = 1.0f / averageDelta;
			}

			// Store the current values as historical ones
			deltaHistory[index] = deltaSeconds;
		}

		void Timer::Tick() {
			updateCount++;

			uint64 now = source->GetTimeMicro();
			if(now < lastTime) {
				now = lastTime;
			}

			deltaMicros = now - lastTime;
			elapsedMicros += deltaMicros;

			deltaSeconds = (now - lastTime) / 1e6f;
			elapsedSeconds = elapsedMicros / 1e6f;
		
			lastTime = now;
			UpdateHistory();
		}

	} // namespace Core

} // namespace Maki
