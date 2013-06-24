#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTimeSource.h"

namespace Maki
{
	namespace Core
	{
		namespace Win32
		{

			class TimeSourceImpl : public TimeSource
			{
			public:
				TimeSourceImpl();
				virtual ~TimeSourceImpl();

				int64 GetTimeMillis();

			private:
				double frequency;
				int64 start;
			};

		} // namespace Win32

	} // namespace Core

} // namespace Maki