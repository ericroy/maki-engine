#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class RenderState;
		class DrawCommandList;
		class CoreManagers;

		class MAKI_CORE_API RenderPayload
		{
		public:
			enum Command
			{
				Command_Abort = 0,
				Command_Init,
				Command_Draw,
				Command_Present,
			};

		public:
			RenderPayload() : cmd(Command_Abort), state(nullptr), commands(nullptr) {}
			RenderPayload(Command cmd) : cmd(cmd), state(nullptr), commands(nullptr) {}

		public:
			Command cmd;
			RenderState *state;
			DrawCommandList *commands;
		};

	} // namespace Core

} // namespace Maki