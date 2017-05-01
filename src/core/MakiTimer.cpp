#include "core/MakiTimer.h"
#include "core/MakiTimeSource.h"

namespace maki {
	namespace core {

		timer_t::timer_t(time_source_t *source)
			: source_(source)
		{
			last_time_ = source_->get_time_micro();

			update_count_ = 0;
			average_fps_ = 0.0f;

			delta_seconds_ = 0.0f;
			delta_micros_ = 0;

			elapsed_seconds_ = 0.0;
			elapsed_micros_ = 0;
		}

		void timer_t::update_history()
		{
			uint32_t index = update_count_ % n_frame_average_;

			// If N frames have gone by, calculate new average values
			if(update_count_ > n_frame_average_ && index == 0) {
				float average_delta = 0;
				for(uint32_t i = 0; i < n_frame_average_; i++ ) {
					average_delta += delta_history_[i];
				}
				average_delta /= n_frame_average_;
				average_fps_ = 1.0f / average_delta;
			}

			// Store the current values as historical ones
			delta_history_[index] = delta_seconds_;
		}

		void timer_t::tick() {
			update_count_++;

			uint64_t now = source_->get_time_micro();
			if(now < last_time_) {
				now = last_time_;
			}

			delta_micros_ = now - last_time_;
			elapsed_micros_ += delta_micros_;

			delta_seconds_ = (now - last_time_) / 1e6f;
			elapsed_seconds_ = elapsed_micros_ / 1e6f;
		
			last_time_ = now;
			update_history();
		}

	} // namespace core
} // namespace maki
