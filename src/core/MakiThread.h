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
			static void run_dispatch(thread_t *instance)
			{
				instance->run();
			}

		public:
			thread_t() {}
			virtual ~thread_t() {}
		
			inline void start()
			{
				if(!thread_.joinable()) {
					thread_ = std::thread(run_dispatch, this);
				}
			}

			inline void join()
			{
				if(thread_.joinable()) {
					thread_.join();
				}
			}

			virtual void run() = 0;

		protected:
			std::thread thread_;
		};

	} // namespace core

} // namespace maki
