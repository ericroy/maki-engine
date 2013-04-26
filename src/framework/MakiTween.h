#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{

	template<class T>
	class Tween
	{
	public:
		Tween(T value = 0, float duration = 1.0f)
			: start(value), end(value), current(value), elapsed(duration), duration(duration), done(true)
		{
		}

		virtual ~Tween()
		{
		}

		inline void Update(float dt)
		{
			if(!done) {
				elapsed += dt;
				if(elapsed >= duration) {
					elapsed = duration;
					current = end;
					done = true;
					if(onComplete != nullptr) {
						onComplete(current);
					}
				} else {
					float t = elapsed / duration;
					current = (1.0f-t) * start + t * end;
				}
			}
		}

		inline T Get() const
		{
			return current;
		}

		inline void Set(T value)
		{
			done = false;
			elapsed = 0.0f;
			start = current;
			end = value;
		}

		inline void Set(T value, float duration)
		{
			this->duration = duration;
			Set(value);
		}

		inline void Set(T value, float duration, std::function<void(T)> onComplete)
		{
			this->onComplete = onComplete;
			Set(value, duration);
		}

		inline void SetDuration(float duration)
		{
			this->duration = duration;
		}

		inline bool IsComplete() const
		{
			return done;
		}

	public:
		std::function<void(T)> onComplete;

	private:
		T start;
		T end;
		T current;
		float elapsed;
		float duration;
		bool done;
	};

} // namespace Maki