#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiWindow.h"
#include "core/MakiTimer.h"
#include "core/MakiInputState.h"

namespace Maki
{

	Engine::Engine(Window *window, RenderCore *core, const AssetLibrary *assets, const Config *config)
		: PseudoSingleton<Engine>(),
		window(window),
		config(config),
		assets(assets),
		renderer(nullptr),
		timer(nullptr),
		inputState(nullptr)
	{
		timer = new Timer();
		inputState = new InputState();
		renderer = new Renderer(window, core, config);
	}

	Engine::~Engine()
	{
		SAFE_DELETE(renderer);
		SAFE_DELETE(inputState);
		SAFE_DELETE(timer);
	}

	void Engine::Update()
	{
		window->PollInput(inputState);
		timer->Tick();
		if(FrameUpdate != nullptr) {
			FrameUpdate();
		}
	}

	void Engine::Draw()
	{
		if(FrameDraw != nullptr) {
			FrameDraw();
		}
	}

} // namespace Maki