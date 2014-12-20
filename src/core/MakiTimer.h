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
			static const int32 n_frame_average_ = 30;

		public:
			timer_t(time_source_t *source);
			void tick();

		private:
			void update_history();

		public:
			uint64 delta_micros_;
			float delta_seconds_;

			double elapsed_seconds_;
			uint64 elapsed_micros_;

			float average_fps_;

		private:
			uint64 last_time_;
			uint64 update_count_;
			float delta_history_[n_frame_average_];
			time_source_t *source_;
		};


	} // namespace core

} // namespace maki
