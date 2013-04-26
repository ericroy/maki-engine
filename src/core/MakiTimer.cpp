#include "core/core_stdafx.h"
#include "core/MakiTimer.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <ctime>
#endif

namespace Maki
{

	Timer::Timer() {
#if defined(_WIN32) || defined(_WIN64)
		if(!QueryPerformanceFrequency((LARGE_INTEGER *)&frequency)) {
			Console::Error("Failed to query performance frequency: %d", GetLastError());
		}
		if(!QueryPerformanceCounter((LARGE_INTEGER *)&lastTime)) {
			Console::Error("Failed to query performance counter: %d", GetLastError());
		}
#else
		lastTime = clock();
#endif

		updateCount = 0;
		averageFps = 0.0f;
		averageDelta = 0.0f;
		deltaSeconds = 0.0f;
		elapsedSeconds = 0.0;
	}

#if defined(_WIN32) || defined(_WIN64)
	void Timer::Tick() {
		updateCount++;

		int64 now;
		if(!QueryPerformanceCounter((LARGE_INTEGER *)&now)) {
			Console::Error("Failed to query performance counter: %d", GetLastError());
		}
		if(now < lastTime) {
			now = lastTime;
		}

		deltaSeconds = (now - lastTime) / (float)frequency;
		elapsedSeconds += deltaSeconds;
		fps = 1.0f / deltaSeconds;
		lastTime = now;

		UpdateHistory();
	}
#else
	void Timer::Tick() {
		updateCount++;

		clock_t now = clock();
		deltaSeconds = (now - lastTime) / (float)CLOCKS_PER_SEC;
		elapsedSeconds += deltaSeconds;
		fps = 1.0f / deltaSeconds;
		lastTime = now;

		UpdateHistory();
	}
#endif

} // namespace Maki
