#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiWindow.h"
#include "core/MakiTimeSource.h"
#include "core/MakiTimer.h"
#include "core/MakiInputState.h"
#include "core/MakiConfig.h"

namespace Maki
{
	namespace Core
	{

		Engine::Engine(Window *window, TimeSource *timeSource, RenderCore *core, const AssetLibrary *assets, const Config *config)
			: PseudoSingleton<Engine>(),
			window(window),
			config(config),
			assets(assets),
			renderer(nullptr),
			timeSource(timeSource),
			updateTimer(timeSource),
			renderTimer(timeSource),
			inputState(nullptr)
		{
			inputState = new InputState();
			renderer = new Renderer(window, core, config);
		
			millisPerUpdate = 1000 / config->GetUint("engine.updates_per_second", DEFAULT_UPDATES_PER_SECOND);
			maxSkippedFrames = config->GetUint("engine.max_skipped_frames", DEFAULT_MAX_SKIPPED_FRAMES);

			nextUpdate = timeSource->GetTimeMillis() + millisPerUpdate;
		}

		Engine::~Engine()
		{
			SAFE_DELETE(renderer);
			SAFE_DELETE(inputState);
		}

		void Engine::Tick()
		{
			// While we are overdue for the next update...
			int64 now = 0;
			uint32 skippedFrames = 0;
			while((now = timeSource->GetTimeMillis()) > nextUpdate && skippedFrames < maxSkippedFrames) {
						
				updateTimer.Tick();
				//Console::Info("Update tick: %f", updateTimer.averageFps);

				window->PollInput(inputState);
				if(FrameUpdate != nullptr) {
					FrameUpdate(millisPerUpdate / 1000.0f);
				}

				nextUpdate = now + millisPerUpdate;
				skippedFrames++;
			}

			renderTimer.Tick();
			//Console::Info("Render tick: %f", renderTimer.averageFps);
			if(FrameDraw != nullptr) {
				FrameDraw();
			}
		}

	} // namespace Core

} // namespace Maki