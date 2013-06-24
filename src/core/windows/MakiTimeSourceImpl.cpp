#include "core/core_stdafx.h"
#include "core/windows/MakiTimeSourceImpl.h"

#include <windows.h>

namespace Maki
{
	namespace Core
	{
		namespace Win32
		{

			TimeSourceImpl::TimeSourceImpl()
			{
				int64 countsPerSecond = 0;
				if(!QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSecond)) {
					Console::Error("Failed to query performance frequency: %d", GetLastError());
				}

				// Counts per millisecond
				frequency = countsPerSecond / 1000.0;

				if(!QueryPerformanceCounter((LARGE_INTEGER *)&start)) {
					Console::Error("Failed to query performance counter: %d", GetLastError());
				}
			}

			TimeSourceImpl::~TimeSourceImpl()
			{
			}

			int64 TimeSourceImpl::GetTimeMillis()
			{
				int64 now = 0;
				if(!QueryPerformanceCounter((LARGE_INTEGER *)&now)) {
					Console::Error("Failed to query performance counter: %d", GetLastError());
				}
				return int64((now - start) / frequency);
			}

		} // namespace Win32

	} // namespace Core

} // namespace Maki
