#pragma once
#include "core/core_stdafx.h"

namespace Maki
{

	class TimeSource
	{
	public:
		virtual int64 GetTimeMillis() = 0;
	};


} // namespace Maki