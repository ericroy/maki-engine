#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		template<class T>
		inline T clamp(T x, T low, T high)
		{
			if(x < low) return low;
			if(x > high) return high;
			return x;
		}
	
	} // namespace core

} // namespace maki
