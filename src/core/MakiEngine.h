#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"
#include "core/MakiTimer.h"

namespace Maki
{
	namespace Core
	{
	
		class Config;
		class AssetLibrary;
		class Renderer;
		class RenderCoreBase;
		class Window;
		class InputState;
		class Timer;
		class TimeSource;

		
		

		class Engine : public PseudoSingleton<Engine>
		{
		public:
			static const uint32 DEFAULT_UPDATES_PER_SECOND = 60;
			static const uint32 DEFAULT_MAX_SKIPPED_FRAMES = 6;

		public:
			Engine(Window *window, TimeSource *timeSource, RenderCoreBase *core, const AssetLibrary *assets, const Config *config);
			virtual ~Engine();
			void Tick();

		public:
			std::function<void(float)> FrameUpdate;
			std::function<void()> FrameDraw;
			const Config *config;
			const AssetLibrary *assets;
		
			Timer updateTimer;
			Timer renderTimer;
		
			InputState *inputState;
			Renderer *renderer;
			Window *window;

		private:
			TimeSource *timeSource;
			int64 nextUpdate;
			uint32 millisPerUpdate;
			uint32 maxSkippedFrames;
		};

	} // namespace Core

} // namespace Maki