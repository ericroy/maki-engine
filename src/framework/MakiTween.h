#pragma once
#include "framework/framework_stdafx.h"

namespace maki
{
	namespace framework
	{

		template<class T>
		class tween_t
		{
		public:
			tween_t(T value = 0, float duration = 1.0f)
				: start_(value), end_(value), current_(value), elapsed_(duration), duration_(duration), done_(true)
			{
			}

			virtual ~tween_t()
			{
			}

			inline void update(float dt)
			{
				if(!done_) {
					elapsed_ += dt;
					if(elapsed_ >= duration_) {
						elapsed_ = duration_;
						current_ = end_;
						done_ = true;
						if(on_complete_ != nullptr) {
							on_complete_(current_);
						}
					} else {
						float t = elapsed_ / duration_;
						current_ = (1.0f-t) * start_ + t * end_;
					}
				}
			}

			inline T get() const
			{
				return current_;
			}

			inline void set(T value)
			{
				done_ = false;
				elapsed_ = 0.0f;
				start_ = current_;
				end_ = value;
			}

			inline void set(T value, float duration)
			{
				duration_ = duration;
				set(value);
			}

			inline void set(T value, float duration, std::function<void(T)> on_complete)
			{
				on_complete_ = on_complete;
				set(value, duration);
			}

			inline void set_duration(float duration)
			{
				duration_ = duration;
			}

			inline bool is_complete() const
			{
				return done_;
			}

		public:
			std::function<void(T)> on_complete_;

		private:
			T start_;
			T end_;
			T current_;
			float elapsed_;
			float duration_;
			bool done_;
		};

	} // namespace framework

} // namespace maki
