#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class TimeSource
		{
		public:
			virtual int64 GetTimeMillis() = 0;
		};


	} // namespace Core

} // namespace Maki