#pragma once
#include "core/core_stdafx.h"
#include "core/MakiRenderCore.h"

namespace Maki
{
	namespace Core
	{
		class Window;
		class Config;

		class MAKI_CORE_API RenderCoreFactory
		{
		private:
			typedef RenderCore *(*MakiRenderCoreCreateFn)(Window *, const Config *);
			typedef void (*MakiRenderCoreDestroyFn)(RenderCore *);

		private:
			static void *d3dRenderCoreLib;
			static void *oglRenderCoreLib;

		public:
			RenderCoreFactory();
			virtual ~RenderCoreFactory();

			RenderCore *Create(RenderCore::Type type, Window *window, const Config *config);
			void Destroy(RenderCore *core);
		};

	} // namespace Core

} // namespace Maki