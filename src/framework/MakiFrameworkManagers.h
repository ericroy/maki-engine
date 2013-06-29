#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiPseudoSingleton.h"

namespace Maki
{
	namespace Core
	{
		class Config;
	}

	namespace Framework
	{
		class ScriptManager;

		class FrameworkManagers : public Core::PseudoSingleton<FrameworkManagers>
		{
		public:
			FrameworkManagers();
			FrameworkManagers(const Config *config);
			virtual ~FrameworkManagers();
			void DumpManagerStats();
			virtual void ReloadAssets();
			virtual void ReloadAsset(Rid rid);

		public:
			ScriptManager *scriptManager;
		};

	} // namespace Core

} // namespace Maki