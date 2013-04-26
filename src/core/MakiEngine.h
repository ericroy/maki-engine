#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"
#include "core/MakiResourceProvider.h"

namespace Maki
{
	class Config;
	class AssetLibrary;
	class Renderer;
	class RenderCore;
	class Window;
	class InputState;
	class Timer;

	class Engine : public PseudoSingleton<Engine>
	{
	public:
		Engine(Window *window, RenderCore *core, const AssetLibrary *assets, const Config *config);
		virtual ~Engine();
		
		void Update();
		void Draw();

	public:
		std::function<void()> FrameUpdate;
		std::function<void()> FrameDraw;
		const Config *config;
		const AssetLibrary *assets;
		Timer *timer;
		InputState *inputState;
		Renderer *renderer;
		Window *window;
	};


} // namespace Maki