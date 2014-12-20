#pragma once
#include "core/core_stdafx.h"
#include <thread>

namespace maki
{
	namespace core
	{
		

		class thread_t
		{
		private:
			static void RunDispatch(thread_t *instance)
			{
				instance->run();
			}

		public:
			thread_t() {}
			virtual ~thread_t() {}
		
			inline void Start()
			{
				if(!thread.joinable()) {
					thread = std::thread(RunDispatch, this);
				}
			}

			inline void Join()
			{
				if(thread.joinable()) {
					thread.join();
				}
			}

			virtual void run() = 0;

		protected:
			std::thread thread;
		};

	} // namespace core

} // namespace maki
