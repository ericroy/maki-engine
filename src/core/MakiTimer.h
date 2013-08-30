#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
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
			uint64 deltaMicros;
			float deltaSeconds;

			double elapsedSeconds;
			uint64 elapsedMicros;

			float averageFps;

		private:
			uint64 lastTime;
			uint64 updateCount;
			float deltaHistory[N_FRAME_AVERAGE];
			TimeSource *source;
		};


	} // namespace Core

} // namespace Maki