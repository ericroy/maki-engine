#include "core/core_stdafx.h"
#include "core/MakiRenderCore.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgram.h"

namespace Maki
{
	RenderCore::RenderCore()
		: Thread()
	{
	}

	RenderCore::~RenderCore()
	{
	}

	void RenderCore::Run()
	{
		Console::Info("Render core started");

		RenderPayload payload;
		while(true) {
			input.Get(payload);
			if(payload.cmd == RenderPayload::Command_Init) {
				Init();
				Console::Info("Render core initialized");
				output.Put(payload);
			} else if(payload.cmd == RenderPayload::Command_Abort) {
				output.Put(payload);
				break;
			} else if(payload.cmd == RenderPayload::Command_Draw) {
#if MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
				payload.commands->Sort();
#endif
				Draw(*payload.state, *payload.commands);
				output.Put(payload);
			} else if(payload.cmd == RenderPayload::Command_Present) {
				Present();
				output.Put(payload);
			}
		}

		Console::Info("Render core stopping");
	}


} // namespace Maki