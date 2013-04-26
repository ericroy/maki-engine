#pragma once
#include "core/core_stdafx.h"
#include <thread>

namespace Maki
{

	class Thread
	{
	public:
		Thread() {}
		virtual ~Thread() {}
		
		inline void Start()
		{
			if(!thread.joinable()) {
				thread = std::thread([this] {
					Run();
				});
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

} // namespace Maki