#pragma once
#include "core/core_stdafx.h"

namespace Maki
{

	class Timer
	{
	public:
		static const int32 N_FRAME_AVERAGE = 30;

	public:
		Timer();
		void Tick();

	private:
		inline void UpdateHistory()
		{
			uint32 index = updateCount % N_FRAME_AVERAGE;

			// If N frames have gone by, calculate new average values
			if(updateCount > N_FRAME_AVERAGE && index == 0) {
				averageDelta = 0;
				for(uint32 i = 0; i < N_FRAME_AVERAGE; i++ ) {
					averageDelta += deltaHistory[i];
				}
				averageDelta /= N_FRAME_AVERAGE;
				averageFps = 1.0f / averageDelta;
			}

			// Store the current values as historical ones
			deltaHistory[index] = deltaSeconds;
		}

	public:
		int64 lastTime;
		uint32 updateCount;

		double elapsedSeconds;
		float deltaSeconds;
		float averageDelta;
		
		float fps;
		float averageFps;

		float deltaHistory[N_FRAME_AVERAGE];

#if defined(_WIN32) || defined(_WIN64)
		int64 frequency;
#endif
	};


} // namespace Maki