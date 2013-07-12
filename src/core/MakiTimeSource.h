#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API TimeSource
		{
		public:
			TimeSource();
			virtual ~TimeSource();

			uint64 GetTimeMillis();

		private:
			double frequency;
			uint64 start;
		};


	} // namespace Core

} // namespace Maki