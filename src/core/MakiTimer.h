#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
		class time_source_t;

		class timer_t
		{
		public:
			static const int32 N_FRAME_AVERAGE = 30;

		public:
			timer_t(time_source_t *source);
			void tick();

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
			time_source_t *source;
		};


	} // namespace core

} // namespace maki
