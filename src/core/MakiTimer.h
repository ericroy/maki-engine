#pragma once
#include "core/MakiTypes.h"
#include "core/MakiMacros.h"

namespace maki {
	namespace core {
		class time_source_t;

		class timer_t {
			MAKI_NO_COPY(timer_t);

		private:
			static const int32_t n_frame_average_ = 30;

		public:
			timer_t(time_source_t *source);
			void tick();
			inline float average_fps() const { return average_fps_; }
			inline float delta_seconds() const { return delta_seconds_; }

		private:
			void update_history();

		private:
			uint64_t delta_micros_;
			float delta_seconds_;
			double elapsed_seconds_;
			uint64_t elapsed_micros_;
			float average_fps_;
			uint64_t last_time_;
			uint64_t update_count_;
			float delta_history_[n_frame_average_];
			time_source_t *source_;
		};


	} // namespace core
} // namespace maki
