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

		MAKI_CORE_API void *MakiLoadLibrary(const char *libName);
		MAKI_CORE_API void MakiFreeLibrary(void *lib);
		MAKI_CORE_API void *MakiGetProcAddress(void *lib, const char *procName);
	
	} // namespace Core

} // namespace Maki