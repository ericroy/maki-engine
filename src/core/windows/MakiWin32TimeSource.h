#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

namespace Maki
{

	class Win32TimeSource : public TimeSource
	{
	public:
		Win32TimeSource();
		virtual ~Win32TimeSource();

		int64 GetTimeMillis();

	private:
		double frequency;
		int64 start;
	};


} // namespace Maki