#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class TimeSource
		{
		public:
			TimeSource();
			virtual ~TimeSource();

			uint64 GetTimeMicro();

		private:
			double frequency;
			uint64 start;
		};


	} // namespace Core

} // namespace Maki