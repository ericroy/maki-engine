#pragma once
#include "core/core_stdafx.h"
#include <thread>

namespace Maki
{
	namespace Core
	{
		MAKI_CORE_API_EXTERN class MAKI_CORE_API std::thread;

		class MAKI_CORE_API Thread
		{
		private:
			static void RunDispatch(Thread *instance)
			{
				instance->Run();
			}

		public:
			Thread() {}
			virtual ~Thread() {}
		
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

			virtual void Run() = 0;

		protected:
			std::thread thread;
		};

	} // namespace Core

} // namespace Maki