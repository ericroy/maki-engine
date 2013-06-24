#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		template<class T>
		inline T Clamp(T x, T low, T high)
		{
			if(x < low) return low;
			if(x > high) return high;
			return x;
		}

	
	} // namespace Core

} // namespace Maki