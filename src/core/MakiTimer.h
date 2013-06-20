#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class TimeSource;

	class Timer
	{
	public:
		static const int32 N_FRAME_AVERAGE = 30;

	public:
		Timer(TimeSource *source);
		void Tick();

	private:
		void UpdateHistory();

	public:
		int64 deltaMillis;
		float deltaSeconds;

		double elapsedSeconds;
		int64 elapsedMillis;

		float averageFps;

	private:
		int64 lastTime;
		int64 updateCount;
		float deltaHistory[N_FRAME_AVERAGE];
		TimeSource *source;
	};


} // namespace Maki